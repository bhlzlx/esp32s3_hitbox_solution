#pragma once
#include <chrono>
#include "idisplay.h"

class StandardDisplay : public IDisplay {
public:
    enum class display_mode_t {
        presplash,
        splash,
        input,
    };
    display_mode_t                          _mode;
private:
    uint8_t                                 _homePin;
    uint8_t                                 _lsPin;
    uint8_t                                 _rsPin;
    uint8_t                                 _step;
    std::string                             _wiredMode;
    std::string                             _batteryLevel;
    std::string                             _socd[4];
    TimeHelper                              _sleepHelper;
    int64_t                                 _sleepElapsed;
public:
    StandardDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2, pin_infos_t* infos);

    virtual IDisplay* update(pin_infos_t * info) override;

    void setWiredMode(WiredMode mode) {
        if(mode == WiredMode::Wired) {
            _wiredMode = "USB";
        } else {
            _wiredMode = "BLE";
        }
    }
    void setBatteryLevel(int level) {
        _batteryLevel = std::to_string(level);
    }
    void sleepAfter(int64_t ms);
    void checkSerialCode();
};