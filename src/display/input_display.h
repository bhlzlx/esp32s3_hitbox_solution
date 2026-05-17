#pragma once

#include <chrono>
#include <set>
#include "idisplay.h"

class InputDisplay : public IDisplay {
private:
    std::set<uint8_t>       _pinRec;
    VBtn                    _vbtnList[19];
    uint8_t                 _vbtnIdx;
    uint8_t                 _homePin;
    uint8_t                 _homeAvail;
public:
    InputDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2, pin_infos_t* infos);
    virtual IDisplay* update(pin_infos_t* infos) override;
};