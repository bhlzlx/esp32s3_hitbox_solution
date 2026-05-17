#pragma once

#include <chrono>
#include <set>
#include "idisplay.h"

class ExtButtonDisplay : public IDisplay {
private:
    std::set<uint8_t>       _extPins;
    std::set<uint8_t>       _standardPins;
    std::set<VBtn>          _pressedVBtn;
    uint8_t                 _homePin;
    bool                    _homeAvail;
public:
    ExtButtonDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2, pin_infos_t* infos);
    virtual IDisplay* update(pin_infos_t* infos) override;
};