#include "layout_display.h"
#include "input_display.h"

LayoutDisplay::LayoutDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2, pin_infos_t* infos)
    : IDisplay(u8g2, display_type_t::layout)
    , _remapOrder {
        (uint8_t)VBtn::HOME,
        (uint8_t)VBtn::START,
        (uint8_t)VBtn::LEFT,
        (uint8_t)VBtn::DOWN,
        (uint8_t)VBtn::RIGHT,
        (uint8_t)VBtn::UP,
        (uint8_t)VBtn::A,
        (uint8_t)VBtn::B,
        (uint8_t)VBtn::RT,
        (uint8_t)VBtn::LT,
        (uint8_t)VBtn::X,
        (uint8_t)VBtn::Y,
        (uint8_t)VBtn::RB,
        (uint8_t)VBtn::LB,
        (uint8_t)VBtn::LS,
        (uint8_t)VBtn::RS,
        (uint8_t)VBtn::BACK,
        (uint8_t)VBtn::EMPTY,
        (uint8_t)VBtn::RT + 1,
    }
{
    configDefault();
    for(uint8_t i = 1; i<MaxPinNumber; ++i) {
        if(infos->vbtns[i][0] != VBtn::INVALID) {
            _pinRec.insert(i);
            pinMode(i, INPUT_PULLUP);
        }
    }
    _orderIndex = 0;
}

IDisplay* LayoutDisplay::update(pin_infos_t* infos) {
    IDisplay* rst = this;
    if(elapsed() > 500) {
        _u8g2->clearBuffer();
        if(_orderIndex >= ARRAY_SIZE(_remapOrder) || _pinRec.empty()) {
            _u8g2->setFont(u8g2_font_spleen5x8_mr);
            {
                auto msg = "layout remapping completed";
                auto width = _u8g2->getStrWidth(msg);
                _u8g2->drawStr((128 - width) / 2, 24, msg);
            }
            {
                auto msg = "wait for key mapping...";
                auto width = _u8g2->getStrWidth(msg);
                _u8g2->drawStr((128 - width) / 2, 36, msg);
            }
            rst = new InputDisplay(_u8g2, infos);
        } else {
            auto currentIdx = _remapOrder[_orderIndex];
            for(uint8_t i = 0; i<ARRAY_SIZE(_remapOrder); ++i) {
                auto posIdx = _remapOrder[i];
                auto& pos = BtnOffsets[posIdx];
                if(posIdx == currentIdx) {
                    _u8g2->drawDisc(pos.x, pos.y, 7);
                } else {
                    _u8g2->drawCircle(pos.x, pos.y, 7);
                }
            }
            for(auto pin: _pinRec) {
                if(!digitalRead(pin)) {
                    infos->displayIdx[pin] = currentIdx;
                    infos->registed[pin] = true;
                    _pinRec.erase(pin);
                    ++_orderIndex;
                    break;
                }
            }
        }
        _u8g2->sendBuffer();
    }
    return rst;
}