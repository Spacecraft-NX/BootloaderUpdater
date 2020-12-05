#ifndef __GWDFU_H__
#define __GWDFU_H__

#include "Serial.h"

enum ERRORS
{
    OKAY = 0x900D0000,
    OKAY_OB_SET_OR_HEADER_GOOD = 0x900D0002,
    OKAY_OB_READ = 0x900D0004,

    ERROR_GENERIC = 0xBAD00000,
    ERROR_KEY_CHECK_IS_NOT_DONE = 0xBAD00008,
    ERROR_8000150_ALREADY_BURNT = 0xBAD00009
};

enum LEDS
{
    RED,
    GREEN,
    BLUE
};

#pragma pack(push, 1)

struct fw_update_header
{
    uint32_t offset;
    uint32_t size;
    uint32_t version;
    uint32_t unk;
    uint8_t hash[16];
};

#pragma pack(pop)

class GWDFU : public Serial
{
public:
    GWDFU(std::string port) : Serial(port) { }

    bool ping();

    bool send_firmware_header(fw_update_header &header);

    void get_current_offset();
    void get_crc();

    void read_uid();

    void set_led_color(LEDS led, uint8_t color);

    bool keycheck();

    void read_8000150();
    void burn_8000150(uint8_t* value);

    void read_8000168();

    void read_8003158();

    void burn_8000160(uint32_t value);
    void read_8000160();

    /*
                    devboard    SX
        OB_SPC      0x5aa5      0x41be
        OB_USER     0x00ff      0x20df
        OB_DATA0    0x00ff      0xffff
        OB_DATA1    0x00ff      0xffff
        OB_WP0      0x00ff      0xffff
        OB_WP1      0x00ff      0xffff
    */

    void read_ob();
    void set_ob(uint8_t ob);
    bool send_data(uint8_t* buffer);
};

#endif