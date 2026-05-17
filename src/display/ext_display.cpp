#include "ext_display.h"
#include "standard_display.h"

ExtButtonDisplay::ExtButtonDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2, pin_infos_t* infos)
    : IDisplay(u8g2, display_type_t::ext)
{
    for(uint8_t i = 1; i<MaxPinNumber; ++i) {
        if(
            infos->vbtns[i][0] != VBtn::INVALID &&
            infos->vbtns[i][0] != VBtn::HOME &&
            infos->vbtns[i][0] != VBtn::START
        ) {
            _standardPins.insert(i);
        } else if(infos->vbtns[i][0] == VBtn::HOME) {
            _homePin = i;
        } else if( infos->registed[i] && infos->vbtns[i][0] == VBtn::INVALID) {
            _extPins.insert(i);
        }
    }
    _homeAvail = true;
}

IDisplay* ExtButtonDisplay::update(pin_infos_t* infos) {
    IDisplay* rst = this;
    if(elapsed() > 200) {
        auto extPin = *_extPins.begin();
        _u8g2->clearBuffer();
        if(_extPins.empty()) {
            auto msg = "ENJOY GAME!";
            auto width = _u8g2->getStrWidth(msg);
            _u8g2->drawStr(32, (128 - width) / 2, msg);
            infos->registed.fill(0);
            infos->writePreference();
            rst = new StandardDisplay(_u8g2, infos);
        } else {
            if(!_homeAvail) {
                if(digitalRead(_homePin)) {
                    _homeAvail = true;
                }
            }
            for(uint8_t i = 1; i<MaxPinNumber; ++i) {
                if(infos->registed[i]) {
                    auto& pos = BtnOffsets[infos->displayIdx[i]];
                    if(i == extPin) {
                        _u8g2->drawDisc(pos.x, pos.y, 7);
                    } else {
                        _u8g2->drawCircle(pos.x, pos.y, 7);
                    }
                }
            }
            for(auto stdPin: _standardPins) {
                if(!digitalRead(stdPin)) {
                    VBtn vbtn = infos->vbtns[stdPin][0];
                    _pressedVBtn.insert(vbtn);
                }
            }
            if((_homeAvail && !digitalRead(_homePin)) || _pressedVBtn.size() >= 4) {
                if(!digitalRead(_homePin)) {
                    _homeAvail= false;
                }
                uint8_t counter = 0;
                for(auto vbtn: _pressedVBtn) {
                    infos->vbtns[extPin][counter] = vbtn;
                    ++counter;
                    if(counter >= MaxBtnPerKey) {
                        break;
                    }
                }
                if(_pressedVBtn.size() == 1) {
                    infos->vbtns[extPin][1] = infos->vbtns[extPin][0];
                    ++counter;
                }
                _extPins.erase(extPin);
                infos->registed[extPin] = false;
                _pressedVBtn.clear();
                delay(400);
            }
        }
        _u8g2->sendBuffer();
    }
    return rst;
}