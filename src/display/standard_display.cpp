#include "standard_display.h"

uint64_t IDisplay::_mac;
uint64_t IDisplay::_serialNumber;

StandardDisplay::StandardDisplay(U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2, pin_infos_t* infos)
    : IDisplay(u8g2, display_type_t::standard)
    , _socd{
        "N",
        "L",
        "F",
        "U"
    }
    , _sleepHelper("screen sleep")
    , _sleepElapsed(INT64_MAX)
{
    _mode = display_mode_t::presplash;
    for(uint8_t i = 1; i<MaxPinNumber; ++i) {
        switch(infos->vbtns[i][0]) {
            case VBtn::HOME: {
                _homePin = i;
                break;
            }
            case VBtn::LS: {
                _lsPin = i;
                break;
            }
            case VBtn::RS: {
                _rsPin = i;
                break;
            }
            default:{
                break;
            }
        }
    }
    _step = 0;
    _sleepHelper.updateTimepoint();
}

void StandardDisplay::checkSerialCode() {
    for(size_t pin = 1; pin<MaxPinNumber; ++pin) {
        if(pinInfos.vbtns[pin][0] != VBtn::INVALID) {
            pinInfos.registed[pin] = 1;
        }
        if(pinInfos.registed[pin]) {
            loopData.registedPins[loopData.registedPinCount++] = pin;
        }
    }
}

void StandardDisplay::sleepAfter(int64_t ms) {
    _sleepElapsed = ms;
}

IDisplay* StandardDisplay::update(pin_infos_t *info) {
    if(_sleepHelper.elapsed() > _sleepElapsed) {
        _u8g2->setPowerSave(true);
        return nullptr;
    }
    if(_mode == display_mode_t::presplash) {
        _u8g2->clearBuffer();
        _u8g2->drawXBM(0, 0, 128, 64, SplashScreen);
        _u8g2->sendBuffer();
        _mode = display_mode_t::splash;
        _timepoint = std::chrono::steady_clock::now();

        // crack way
        // left right up donw
        #ifdef MY_DEVKIT
        int pins[] = {15, 16, 17, 18};
        #endif
        #ifdef BOARD_NOLOGO
        int pins[] = {13, 14, 15, 16};
        #endif
        int val = 0;
        for(int i = 0; i<4; ++i) {
            if(!digitalRead(pins[i])) {
                val |= (1 << i);
            }
        }
        if(val == 0b1111) {
            auto uuid1 = _mac << 50;
            auto uuid2 = _mac >> 14;
            auto key = uuid1 | uuid2;
            uuid1 = key >> 32;
            uuid2 = key & 0xffffffff;
            key = uuid1 ^ uuid2;
            Preferences pref;
            pref.begin(APP_NS);
            pref.putUInt(SERIAL_KEY, key);
            pref.end();
        }
        // crack code
        checkSerialCode();
    }
    else if(_mode == display_mode_t::splash) {
        if(elapsed() > 2000) {
            _mode = display_mode_t::input;
            _timepoint = std::chrono::steady_clock::now();
        }
    } else if(_mode == display_mode_t::input) {
        // if(elapsed() > 100) {
            if(_step == 0) {
                _u8g2->clearBuffer();
                ++_step;
            } else if(_step <= loopData.registedPinCount) {
                auto pin = loopData.registedPins[_step-1];
                auto& pos = BtnOffsets[info->displayIdx[pin]];
                if(info->pressed[pin]) {
                    _u8g2->drawDisc(pos.x, pos.y, 7);
                    std::string msg = std::to_string(pin);
                    Serial.write(msg.c_str());
                    msg = std::to_string(info->displayIdx[pin]);
                    Serial.write(msg.c_str());
                    msg = std::to_string(pos.x);
                    Serial.write(msg.c_str());
                    msg = std::to_string(pos.y);
                    Serial.write(msg.c_str());
                } else {
                    _u8g2->drawCircle(pos.x, pos.y, 7);
                }
                ++_step;
            } else if(_step == loopData.registedPinCount+1) {
                _u8g2->setFont(u8g2_font_5x8_tf);
                _u8g2->drawStr(0, 43, _socd[(int)loopData.socdMode].c_str());
                _u8g2->drawStr(0, 51, _wiredMode.c_str());
                ++_step;
            } else if(_step == loopData.registedPinCount+2) {
                _u8g2->drawRFrame(0, 55, 14, 8, 2); //, 1(0, 64, _batteryLevel.c_str());
                if(_batteryLevel.size() == 2) {
                    _u8g2->drawStr(3, 62, _batteryLevel.c_str());
                } else {
                    _u8g2->drawStr(1, 62, _batteryLevel.c_str());
                }
                ++_step;
            } else if(_step == loopData.registedPinCount+3) {
                _u8g2->sendBuffer();
                ++_step;
            } else {
                if(elapsed() > 16) {
                    _step = 0;
                    _timepoint = std::chrono::steady_clock::now();
                }
            }

        //     for(int idx = 0; idx< loopData.registedPinCount; ++idx) {
        //         auto pin = loopData.registedPins[idx];
        //         // if(info->registed[pin]) {
        //         auto& pos = BtnOffsets[info->displayIdx[pin]];
        //         if(info->pressed[pin]) {
        //             _u8g2->drawDisc(pos.x, pos.y, 7);
        //             std::string msg = std::to_string(pin);
        //             Serial.write(msg.c_str());
        //             msg = std::to_string(info->displayIdx[pin]);
        //             Serial.write(msg.c_str());
        //             msg = std::to_string(pos.x);
        //             Serial.write(msg.c_str());
        //             msg = std::to_string(pos.y);
        //             Serial.write(msg.c_str());
        //         } else {
        //             _u8g2->drawCircle(pos.x, pos.y, 7);
        //         }
        //         // }
        //     }

        //     _u8g2->setFont(u8g2_font_5x8_tf);
        //     _u8g2->drawStr(0, 43, _socd[(int)loopData.socdMode].c_str());
        //     _u8g2->drawStr(0, 51, _wiredMode.c_str());
        //     _u8g2->drawRFrame(0, 55, 14, 8, 2); //, 1(0, 64, _batteryLevel.c_str());
        //     if(_batteryLevel.size() == 2) {
        //         _u8g2->drawStr(3, 62, _batteryLevel.c_str());
        //     } else {
        //         _u8g2->drawStr(1, 62, _batteryLevel.c_str());
        //     }

        //     _u8g2->sendBuffer();
        // }
    }
    return this;
}
