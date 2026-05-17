#include "input_display.h"
#include "ext_display.h"

#define SKIP_LAYOUT_CONFIG 1

static char VBtnNames[][8] {
    "LB",
    "RB",
    "HOME",
    "EMPTY",
    "A",
    "B",
    "X",
    "Y",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "START",
    "BACK",
    "LS",
    "RS",
    "LT",
    "RT",
    "EMPTY2",
    "UNKNOWN",
    "UNKNOWN",
};

InputDisplay::InputDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2, pin_infos_t* infos)
    : IDisplay(u8g2, display_type_t::input)
    , _vbtnList{
        VBtn::HOME, VBtn::START,
        VBtn::UP, VBtn::DOWN, VBtn::LEFT, VBtn::RIGHT,
        VBtn::A, VBtn::B, VBtn::LT, VBtn::RT,
        VBtn::X, VBtn::Y, VBtn::LB, VBtn::RB, 
        VBtn::LS, VBtn::RS, VBtn::BACK,
        VBtn::EMPTY
    }
{
    #if SKIP_LAYOUT_CONFIG
    configDefault();
    for(uint8_t i = 1; i<MaxPinNumber; ++i) {
        if(infos->vbtns[i][0] != VBtn::INVALID) {
            infos->registed[i] = true;
            _pinRec.insert(i);
            pinMode(i, INPUT_PULLUP);
        }
    }
    #else
    for(uint8_t i = 1; i<MaxPinNumber; ++i) {
        if(infos->registed[i]) {
            _pinRec.insert(i);
        }
    }
    #endif
    for(auto& btns: infos->vbtns) { // 清除所有旧的配置数据
        btns.fill(VBtn::INVALID);
    }
    _vbtnIdx = 0;
    _homePin = 0;
    _homeAvail = false;
}

IDisplay* InputDisplay::update(pin_infos_t* infos) {
    _u8g2->clearBuffer();
    auto vbtn = _vbtnList[_vbtnIdx];
    if(_pinRec.empty() || vbtn == VBtn::EMPTY) {
        _u8g2->setFont(u8g2_font_spleen5x8_mr);

        {
            auto msg = "wait for EXT Mapping!";
            auto width = _u8g2->getStrWidth(msg);
            _u8g2->drawStr((128 - width) / 2, 12, msg);
        }
        _u8g2->sendBuffer();
        return new ExtButtonDisplay(_u8g2, infos);
    } else {
        auto name = VBtnNames[(uint8_t)vbtn];
        _u8g2->setFont(u8g2_font_spleen8x16_mf);
        auto width = _u8g2->getStrWidth(name);
        _u8g2->drawStr((128 - width) / 2, 36, name);
        // for(uint8_t i = 0; i<ARRAY_SIZE(BtnOffsets); ++i) {
        //     auto& pos = BtnOffsets[i];
        //     // _pinRec.find()
        //     // if(i == _displayIdx) {
        //         // _u8g2.drawDisc(pos.x, pos.y, 6);
        //     // } else {
        //         _u8g2->drawCircle(pos.x, pos.y, 4);
        //     // }
        // }
        if(!_homeAvail) {
            if(digitalRead(_homePin)) {
                _homeAvail = true;
            }
        }
        if(vbtn != VBtn::HOME) {
            if(!digitalRead(_homePin) && _homeAvail) {
                ++_vbtnIdx;
                _u8g2->sendBuffer();
                _homeAvail = false;
                return this;
            }
        }

        for(auto pin: _pinRec) {
            if(!digitalRead(pin)) {
                infos->configVBtn(pin, vbtn);
                if(vbtn == VBtn::HOME) {
                    _homePin = pin;
                    _homeAvail = false;
                }
                _pinRec.erase(pin);
                _vbtnIdx++;
                break;
            }
        }
    }
    _u8g2->sendBuffer();
    return this;
}