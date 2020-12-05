#include "GWDFU.h"

enum COMMANDS
{
    PING = 0xFACE0000,

    SEND_FIRMWARE_HEADER = 0xFACE0002,

    GET_CURRENT_OFFSET = 0xFACE0004,
    GET_CRC,
    READ_UID,

    SEND_LED_COLOR = 0xFACE0008,

    KEY_CHECK = 0xFACE000F,
    READ_8000150, // License?
    BURN_8000150,

    READ_8000168 = 0xFACE0031,
    READ_8003158,
    BURN_8000160, // Needs KEY_CHECK
    READ_8000160,

    READ_OB = 0xD15EA5E,
    SET_OB = 0xDEFACED, // Needs KEY_CHECK
};

bool GWDFU::ping()
{
    uint8_t outbuffer[4];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = PING;

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    /*printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");*/

    return *(uint32_t*) inbuffer == OKAY;
}

bool GWDFU::send_firmware_header(fw_update_header& header)
{
    uint8_t outbuffer[4 + sizeof(fw_update_header)];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = SEND_FIRMWARE_HEADER;
    memcpy(&outbuffer[4], &header, sizeof(fw_update_header));

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    /*printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");*/

    return *(uint32_t*)inbuffer == OKAY_OB_SET_OR_HEADER_GOOD;
}


void GWDFU::get_current_offset()
{
    uint8_t outbuffer[4];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = GET_CURRENT_OFFSET;

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");
}

void GWDFU::get_crc()
{
    uint8_t outbuffer[4];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = GET_CRC;

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");
}

void GWDFU::read_uid()
{
    uint8_t outbuffer[4];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = READ_UID;

    write(outbuffer, sizeof(outbuffer));

    printf("%s\n", __func__);
    for (int i = 0; i < 4; i++)
    {
        uint8_t inbuffer[4];
        read(inbuffer, sizeof(inbuffer));

        printf("0x%08x\n", *(uint32_t*)inbuffer);
    }
    printf("\n");
}

void GWDFU::set_led_color(LEDS led, uint8_t color)
{
    uint8_t outbuffer[12];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = SEND_LED_COLOR;
    *(uint32_t*)&outbuffer[4] = led;
    *(uint32_t*)&outbuffer[8] = color;

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");
}

bool GWDFU::keycheck()
{
    uint8_t outbuffer[4 + 12];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = KEY_CHECK;

    // Because of gateway's awesome memcmp we can send all 00. lmao

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    /*printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");*/

    return *(uint32_t*)inbuffer == OKAY;
}

void GWDFU::read_8000150()
{
    uint8_t outbuffer[4];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = READ_8000150;

    write(outbuffer, sizeof(outbuffer));

    printf("%s\n", __func__);
    for (int i = 0; i < 4; i++)
    {
        uint8_t inbuffer[4];
        read(inbuffer, sizeof(inbuffer));

        printf("0x%08x\n", *(uint32_t*)inbuffer);
    }
    printf("\n");
}

void GWDFU::burn_8000150(uint8_t* value)
{
    uint8_t outbuffer[4 + 16];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = BURN_8000150;
    memcpy(&outbuffer[4], value, 16);

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");
}

void GWDFU::read_8000168()
{
    uint8_t outbuffer[4];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = READ_8000168;

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");
}

void GWDFU::read_8003158()
{
    uint8_t outbuffer[4];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = READ_8003158;

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");
}

void GWDFU::burn_8000160(uint32_t value)
{
    uint8_t outbuffer[8];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = BURN_8000160;
    *(uint32_t*)&outbuffer[4] = value;

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");
}

void GWDFU::read_8000160()
{
    uint8_t outbuffer[4];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = READ_8000160;

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");
}

void GWDFU::read_ob()
{
    uint8_t outbuffer[4];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = READ_OB;

    write(outbuffer, sizeof(outbuffer));

    printf("%s\n", __func__);
    for (int i = 0; i < 4; i++)
    {
        uint8_t inbuffer[4];
        read(inbuffer, sizeof(inbuffer));

        printf("0x%08x\n", *(uint32_t*)inbuffer);
    }
    printf("\n");
}

void GWDFU::set_ob(uint8_t ob)
{
    uint8_t outbuffer[8];
    memset(outbuffer, 0, sizeof(outbuffer));

    *(uint32_t*)outbuffer = SET_OB;
    *(uint32_t*)&outbuffer[4] = ob;

    write(outbuffer, sizeof(outbuffer));

    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    printf("%s\n", __func__);
    printf("0x%08x\n", *(uint32_t*)inbuffer);
    printf("\n");
}

bool GWDFU::send_data(uint8_t* buffer)
{
    write(buffer, 64);
    uint8_t inbuffer[4];
    read(inbuffer, sizeof(inbuffer));

    return *(uint32_t*)inbuffer == 0x900D0003 || *(uint32_t*)inbuffer == 0x900D0000;
}