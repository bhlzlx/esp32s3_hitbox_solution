#pragma once
#include <chrono>
#include <set>
#include "idisplay.h"

class LayoutDisplay : public IDisplay {
private:
    std::set<uint8_t>       _pinRec;
    int8_t                  _orderIndex;
    uint8_t                 _remapOrder[19];
public:
    LayoutDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2, pin_infos_t* infos);

    virtual IDisplay* update(pin_infos_t* infos) override;
};
