#pragma once
#include "U8g2lib.h"
#include "../common_def.h"
#include <chrono>

enum class display_type_t {
    standard,
    layout,
    input,
    ext,
    serial,
};

class IDisplay {
protected:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C     *_u8g2;
    std::chrono::steady_clock::time_point   _timepoint;
    display_type_t                          _type;
public:
    static uint64_t                         _mac;
    static uint64_t                         _serialNumber;

    IDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2, display_type_t type)
        : _u8g2(u8g2)
        , _type(type)
        // : _u8g2(U8G2_R0, U8X8_PIN_NONE, sck, sda)
    {
        _timepoint = std::chrono::steady_clock::now();
        _mac = ESP.getEfuseMac();
    }
    virtual ~IDisplay() {
    }

    int64_t elapsed() {
        auto timepast = std::chrono::steady_clock::now() - _timepoint;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepast);
        return ms.count();
    }

    virtual IDisplay* update(pin_infos_t *infos) = 0;

    display_type_t type() const {
        return _type;
    }
};