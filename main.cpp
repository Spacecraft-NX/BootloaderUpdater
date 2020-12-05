#include <Windows.h>
#include <stdio.h>
#include <string>
#include <tchar.h>
#include <string.h>
#include <Setupapi.h>
#include <time.h>
#include "DFU.h"
#include "GWDFU.h"
#include "bootloader_updater.h"

void GetComPort(TCHAR* pszComePort, uint16_t vid, uint16_t pid)
{
    HDEVINFO DeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;
    const char *DevEnum = "USB";
    TCHAR ExpectedDeviceId[80] = { 0 }; //Store hardware id
    BYTE szBuffer[1024] = { 0 };
    DEVPROPTYPE ulPropertyType;
    DWORD dwSize = 0;
    DWORD Error = 0;
    //create device hardware id
    snprintf(ExpectedDeviceId, sizeof(ExpectedDeviceId), "\\??\\USB#VID_%04X&PID_%04X", vid, pid);
    //SetupDiGetClassDevs returns a handle to a device information set
    DeviceInfoSet = SetupDiGetClassDevs(NULL, DevEnum, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (DeviceInfoSet == INVALID_HANDLE_VALUE)
        return;

    ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD DeviceIndex = 0; SetupDiEnumDeviceInfo(DeviceInfoSet, DeviceIndex, &DeviceInfoData); DeviceIndex++)
    {
        if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet, &DeviceInfoData, SPDRP_HARDWAREID, &ulPropertyType, (BYTE*)szBuffer, sizeof(szBuffer), &dwSize))
        {
            HKEY hDeviceRegistryKey = SetupDiOpenDevRegKey(DeviceInfoSet, &DeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
            if (hDeviceRegistryKey == INVALID_HANDLE_VALUE)
            {
                Error = GetLastError();
                break; //Not able to open registry
            }

            char pszSymbolicName[2048];
            DWORD dwSize2 = sizeof(pszSymbolicName);
            DWORD dwType2 = 0;
            if ((RegQueryValueEx(hDeviceRegistryKey, "SymbolicName", NULL, &dwType2, (LPBYTE)pszSymbolicName, &dwSize2) == ERROR_SUCCESS) && (dwType2 == REG_SZ))
            {
                if (_tcsnicmp(pszSymbolicName, ExpectedDeviceId, strlen(ExpectedDeviceId)) == 0)
                {
                    char pszPortName[2048];
                    DWORD dwSize3 = sizeof(pszPortName);
                    DWORD dwType3 = 0;
                    if ((RegQueryValueEx(hDeviceRegistryKey, "PortName", NULL, &dwType3, (LPBYTE)pszPortName, &dwSize3) == ERROR_SUCCESS) && (dwType3 == REG_SZ))
                    {
                        // Check if it really is a com port
                        if (_tcsnicmp(pszPortName, _T("COM"), 3) == 0)
                        {
                            int nPortNr = _ttoi(pszPortName + 3);
                            if (nPortNr != 0)
                            {
                                _tcscpy_s(pszComePort, sizeof(pszPortName), pszPortName);
                            }
                        }
                    }
                }
            }

            RegCloseKey(hDeviceRegistryKey);
        }
    }
    if (DeviceInfoSet)
    {
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }
}

void generate_key(uint32_t k[4], uint32_t offset)
{
    k[0] = 0x73707048 + offset;
    k[1] = 0xE8AD34FB + offset;
    k[2] = 0xA4A8ACE6 + offset;
    k[3] = 0x7B648B68 + offset;
}

void tiny_encrypt(uint32_t v[2], const uint32_t k[4])
{
    uint32_t v0 = v[0], v1 = v[1], sum = 0, i;
    uint32_t delta = 0x9E3779B9;
    uint32_t k0 = k[0], k1 = k[1], k2 = k[2], k3 = k[3];
    for (i = 0; i < 32; i++)
    {
        sum += delta;
        v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
    }
    v[0] = v0; v[1] = v1;
}

void tiny_hash(uint32_t hash[4], uint32_t v[2], const uint32_t k[4])
{
    uint32_t v0 = v[0], v1 = v[1], sum = 0xC6EF3720;
    uint32_t k0 = k[0], k1 = k[1], k2 = k[2], k3 = k[3];

    hash[3] -= (((v0 << 4) + k1) ^ (hash[2] + sum) ^ ((v1 >> 5) + k2));
    hash[2] -= (((v1 << 4) + k3) ^ (hash[3] + sum) ^ ((v0 >> 5) + k0));
    hash[1] -= (((v0 << 4) + k2) ^ (hash[0] + sum) ^ ((v0 >> 5) + k3));
    hash[0] -= (((v1 << 4) + k0) ^ (hash[1] + sum) ^ ((v1 >> 5) + k1));
}

bool gw2updater(TCHAR* comname)
{
    printf("Flashing bootloader updater.");
    GWDFU dfu(comname);

    if (!dfu.ping())
        return false;

    if (!dfu.keycheck())
        return false;

    int len = bootloader_updater_bin_len;
    len += ((0x400 - (len & 0x3ff)) & 0x3ff);
    
    uint8_t* buf = new uint8_t[len];
    memset(buf, 0, len);
    memcpy(buf, bootloader_updater_bin, bootloader_updater_bin_len);

    uint32_t hash[4];
    memset(hash, 0, sizeof(hash));
    for (int i = 0; i < len; i += 8)
    {
        uint32_t key[4];
        generate_key(key, i);
        tiny_hash(hash, (uint32_t*)&buf[i], key);
        tiny_encrypt((uint32_t*)&buf[i], key);
    }

    fw_update_header header;
    header.offset = 0;
    header.size = len;
    header.version = 0x200;
    header.unk = 3;
    memcpy(header.hash, hash, sizeof(hash));
    if (!dfu.send_firmware_header(header))
        return false;

    for (int i = 0; i < len; i += 64)
    {
        printf(".");
        if (!dfu.send_data(&buf[i]))
            return false;
    }
    printf("\n");

    {
        uint8_t zero = 0;
        dfu.write(&zero, 1);
    }

    return true;
}

bool sc2updater(TCHAR* comname)
{
    printf("Flashing bootloader updater.");
    DFU dfu(comname);

    if (!dfu.ping())
        return false;

    if (!dfu.set_offset(0x3000))
        return false;

    for (int i = 0; i < bootloader_updater_bin_len; i += 64)
    {
        printf(".");
        if (!dfu.send_data(&bootloader_updater_bin[i]))
            return false;
    }
    printf("\n");

    {
        uint8_t zero = 0;
        dfu.write(&zero, 1);
    }

    return true;
}

int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <bootloader.bin>", argv[0]);
        return -1;
    }

    TCHAR gw[2048] = { 0 };
    TCHAR sc[2048] = { 0 };
    TCHAR updater[2048] = { 0 };

    GetComPort(gw, 0xC001, 0xC0DE);
    GetComPort(sc, 0x600D, 0xC0DE);
    GetComPort(updater, 0xBAAD, 0xC0DE);

    if (strlen(gw) != 0)
    {
        printf("Gateway DFU found!\n");
        if (!gw2updater(gw))
        {
            printf("\nFailed to flash bootloader updater!\n");
            return -1;
        }
    }
    else if (strlen(sc) != 0)
    {
        printf("Spacecraft-NX DFU found!\n");
        if (!sc2updater(sc))
        {
            printf("\nFailed to flash bootloader updater!\n");
            return -1;
        }
    }
    else if (strlen(updater) == 0)
    {
        printf("No device detected!\n");
        return -1;
    }

    // Let windows detect the updater
    if (strlen(updater) == 0)
    {
        printf("Waiting for the updater to boot...\n");
        for (int i = 0; i < 50 && strlen(updater) == 0; i++)
        {
            Sleep(100);
            GetComPort(updater, 0xBAAD, 0xC0DE);
        }
    }

    if (strlen(updater) == 0)
    {
        printf("Timed out!\n");
        return -1;
    }

    DFU dfu(updater);

    {
        time_t t = time(0);
        tm* now = localtime(&t);
        char path[MAX_PATH];
        snprintf(path, MAX_PATH, "backup_%04d_%02d_%02d_%02d_%02d_%02d.bin", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

        FILE* f = fopen(path, "wb");
        if (!f)
        {
            printf("Failed to open %s!\n", path);
            return -1;
        }
        uint8_t buffer[64];
        printf("Backing up the current bootloader.\n");
        for (int i = 0; i < 0x3000; i += 64)
        {
            printf(".");
            dfu.read_flash(i, buffer, sizeof(buffer));
            fwrite(buffer, 1, 64, f);
        }
        printf("\n");
        fclose(f);
    }

    {
        dfu.set_offset(0);

        FILE* f = fopen(argv[1], "rb");
        if (!f)
        {
            printf("Failed to open %s!\n", argv[1]);
            return -1;
        }
        printf("Flashing the new bootloader.\n");

        uint8_t buffer[64];
        while (fread(buffer, 1, 64, f) > 0)
        {
            printf(".");
            dfu.send_data(buffer);
        }
        printf("\n");

        fclose(f);
    }

    {
        uint8_t zero = 0;
        dfu.write(&zero, 1);
    }
    
    return 0;
}
