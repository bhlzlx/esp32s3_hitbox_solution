#pragma once

#include <BleCompositeHID.h>
#include <BleConnectionStatus.h>
#include <XboxGamepadDevice.h>
#include <Preferences.h>
#include <chrono>
#include <set>
#include <map>
#include <vector>
#include <string>
#include "socd.h"

// constexpr int MaxBtnPerKey = 4;
#define MaxBtnPerKey 4
#define MaxPinNumber 64
#define APP_NS "esp32"
#define SERIAL_KEY "advance"
#define PIN_CONFIG_KEY "pin"
#define GENERAL_CONFIG_KEY "general"

struct point_t {
    int x;
    int y;
};

enum class VBtn {
    LB,
    RB,
    HOME,
    EMPTY,
    A,
    B,
    X,
    Y,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    START,
    BACK,
    LS,
    RS,
    LT,
    RT,
    VBTN_COUNT,
    INVALID,
};

struct pin_infos_t {
    std::array<uint8_t, MaxPinNumber>               pressed;
    std::array<uint8_t, MaxPinNumber>               displayIdx;
    std::array<uint8_t, MaxPinNumber>               registed;
    std::array<std::array<VBtn, 4>, MaxPinNumber>   vbtns;
    // std::set<uint8_t>                               defaultPins;

    bool                                            autoLayout;

    void reset() {
        pressed.fill(0);
        displayIdx.fill(0);
        registed.fill(0);
        for(auto &vbtnsv: vbtns) {
            vbtnsv.fill(VBtn::INVALID);
        }
        // defaultPins.clear();
    }

    bool loadPreference() {
        Preferences pref;
        pref.begin(APP_NS);
        size_t bytes = pref.getBytes(PIN_CONFIG_KEY, this, sizeof(pin_infos_t));
        pref.end();

        if(bytes) {
            for(uint8_t pin = 1; pin<MaxPinNumber; ++pin) {
                if(vbtns[pin][0] != VBtn::INVALID) {
                    // defaultPins.insert(pin);
                }
            }
            return true;
        } else {
            return false;
        }
    }

    uint8_t getPin(VBtn btn) {
        for(uint8_t i = 1; i<MaxPinNumber; ++i) {
            if(vbtns[i][0] == btn) {
                return i;
            }
        }
    }

    void writePreference() {
        Preferences pref;
        pref.begin(APP_NS);
        size_t bytes = pref.putBytes(PIN_CONFIG_KEY, this, sizeof(pin_infos_t));
        pref.end();
    }

    template<typename ...ARGS>
    void configVBtn(uint8_t pin, ARGS&&  ...args) {
        pinMode(pin, INPUT_PULLUP);
        using BtnType = typename std::common_type<ARGS...>::type;
        constexpr size_t nargs = sizeof...(args);
        auto btns = std::array<BtnType, MaxBtnPerKey>{std::forward<ARGS>(args)...};
        if(nargs < 4) {
            btns[nargs] = VBtn::INVALID;
        }
        vbtns[pin] = btns;
        if(autoLayout) {
            displayIdx[pin] = (uint8_t)vbtns[pin][0];
        }
    }
};

struct general_config_t {
    SOCD::mode_t        socd;
    bool loadPreference() {
        Preferences pref;
        pref.begin(APP_NS);
        size_t bytes = pref.getBytes(GENERAL_CONFIG_KEY, this, sizeof(*this));
        pref.end();
        if(bytes) {
            return true;
        } else {
            return false;
        }
    }

    void writePreference() {
        Preferences pref;
        pref.begin(APP_NS);
        size_t bytes = pref.putBytes(GENERAL_CONFIG_KEY, this, sizeof(*this));
        pref.end();
    }
};

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// static uint8_t registed_pins[] = {
//     6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 1, 2, 41, 40, 36, 47

// };
// constexpr int registed_pin_count = ARRAY_SIZE(registed_pins);


static uint32_t XinputToXBOXButton[]{
    XBOX_BUTTON_LB,
    XBOX_BUTTON_RB,
    XBOX_BUTTON_HOME,
    XBOX_BUTTON_SHARE,
    XBOX_BUTTON_A,
    XBOX_BUTTON_B,
    XBOX_BUTTON_X,
    XBOX_BUTTON_Y,
    XBOX_BUTTON_DPAD_NORTH,
    XBOX_BUTTON_DPAD_SOUTH,
    XBOX_BUTTON_DPAD_WEST,
    XBOX_BUTTON_DPAD_EAST,
    XBOX_BUTTON_START,
    XBOX_BUTTON_SELECT,
    XBOX_BUTTON_LS,
    XBOX_BUTTON_RS,
};

static XboxDpadFlags XinputToXBOXDpad[]{
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NORTH,
    XboxDpadFlags::SOUTH,
    XboxDpadFlags::WEST,
    XboxDpadFlags::EAST,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
    XboxDpadFlags::NONE,
};

// static uint8_t VBtnLayoutMapping[(int)VBtn::VBTN_COUNT] = {};

enum WiredMode
{
    Wired,
    Wireless
};

#include "splash.h"

#include "button_layout.h"

class TimeHelper {
    std::chrono::steady_clock::time_point       _timepoint;
    char const *                                _name;
public:
    TimeHelper(char const* n)
        : _name(n)
    {
        updateTimepoint();
    }
    ~TimeHelper() {
        // Serial.write(_name);
        // Serial.write(':');
        // Serial.write(std::to_string(elapsed()).c_str());
        // Serial.write('\n');
    }
    //
    int64_t elapsed() {
        auto diff = std::chrono::steady_clock::now() - _timepoint;
        auto mills = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
        return mills.count();
    }

    void updateTimepoint() {
        _timepoint = std::chrono::steady_clock::now();
    }
};

struct loop_data_t {
    std::array<std::array<uint8_t, (int)VBtn::VBTN_COUNT>, 2>           vbtnStates;
    uint8_t                                                             idx;
    uint8_t                                                             registedPins[MaxPinNumber] = {};
    uint32_t                                                            registedPinCount = 0;
    SOCD                                                                socd;
    SOCD::mode_t                                                        socdMode = SOCD::mode_t::natural;
    uint8_t                                                             inputChanged:1;

    void reset() {
        memset(&vbtnStates, 0, sizeof(vbtnStates));
        registedPinCount = 0;
        socd.reset();
        idx = 0;
    }

    uint8_t getInputState(int vbtn) {
        return vbtnStates[idx][vbtn];
    }

    void updateInputState(pin_infos_t* pinInfos) {
        vbtnStates[idx].fill(0);
        for(uint8_t i = 0; i < registedPinCount; ++i) {
            auto pin = registedPins[i];
            auto const &vbtns = pinInfos->vbtns[pin];
            if (!digitalRead(pin))
            {
                Serial.write(std::to_string(pin).c_str());
                pinInfos->pressed[pin] = true;
                for(auto vbtn: vbtns) {
                    if(vbtn != VBtn::INVALID) {
                        vbtnStates[idx][(int)vbtn] |= 1;
                    } else {
                        break;
                    }
                }
            } else {
                pinInfos->pressed[pin] = false;
            }
        }
        // apply SOCD
        socd.query(
            (bool&)vbtnStates[idx][(int)VBtn::UP],
            (bool&)vbtnStates[idx][(int)VBtn::DOWN],
            (bool&)vbtnStates[idx][(int)VBtn::LEFT],
            (bool&)vbtnStates[idx][(int)VBtn::RIGHT],
            socdMode
        );
        inputChanged = vbtnStates[0] != vbtnStates[1];
    }

    void swapBuffer() {
        idx ^= 1;
    }
};

extern pin_infos_t pinInfos;
extern loop_data_t loopData;
extern general_config_t generalConfig;

static void configDefault() {
    pinInfos.reset();
    pinInfos.autoLayout = true;
    #ifdef MY_DEVKIT
    pinInfos.configVBtn(6, VBtn::BACK);
    pinInfos.configVBtn(2, VBtn::START);
    pinInfos.configVBtn(1, VBtn::HOME);
    pinInfos.configVBtn(7, VBtn::UP);
    pinInfos.displayIdx[7] = (int)VBtn::EMPTY;
    pinInfos.configVBtn(21, VBtn::A, VBtn::B);
    pinInfos.displayIdx[21] = (int)VBtn::RT+1;

    pinInfos.configVBtn(15, VBtn::UP);
    pinInfos.configVBtn(16, VBtn::DOWN);
    pinInfos.configVBtn(18, VBtn::RIGHT);
    pinInfos.configVBtn(17, VBtn::LEFT);

    pinInfos.configVBtn(9, VBtn::A);
    pinInfos.configVBtn(10, VBtn::B);
    pinInfos.configVBtn(37, VBtn::LT);
    pinInfos.configVBtn(38, VBtn::RT);
    pinInfos.configVBtn(11, VBtn::X);
    pinInfos.configVBtn(12, VBtn::Y);
    pinInfos.configVBtn(35, VBtn::LB);
    pinInfos.configVBtn(36, VBtn::RB);

    pinInfos.configVBtn(8, VBtn::LS);
    pinInfos.configVBtn(3, VBtn::RS);
    #endif
    // nologo
    #ifdef BOARD_NOLOGO
    pinInfos.configVBtn(42, VBtn::BACK);
    pinInfos.configVBtn(41, VBtn::START);
    pinInfos.configVBtn(4, VBtn::HOME);
    pinInfos.configVBtn(5, VBtn::A, VBtn::B);
    pinInfos.displayIdx[5] = (int)VBtn::EMPTY;
    pinInfos.configVBtn(39, VBtn::UP);
    pinInfos.displayIdx[39] = (int)VBtn::RT+1;

    pinInfos.configVBtn(13, VBtn::UP);
    pinInfos.configVBtn(14, VBtn::DOWN);
    pinInfos.configVBtn(15, VBtn::RIGHT);
    pinInfos.configVBtn(16, VBtn::LEFT);

    pinInfos.configVBtn(17, VBtn::A);
    pinInfos.configVBtn(18, VBtn::B);
    pinInfos.configVBtn(33, VBtn::LT);
    pinInfos.configVBtn(34, VBtn::RT);
    pinInfos.configVBtn(35, VBtn::X);
    pinInfos.configVBtn(36, VBtn::Y);
    pinInfos.configVBtn(37, VBtn::LB);
    pinInfos.configVBtn(38, VBtn::RB);

    pinInfos.configVBtn(1, VBtn::LS);
    pinInfos.configVBtn(2, VBtn::RS);
    #endif

    #ifdef BOARD_WALNUT

    pinInfos.configVBtn(15, VBtn::UP);
    pinInfos.configVBtn(16, VBtn::DOWN);
    pinInfos.configVBtn(17, VBtn::RIGHT);
    pinInfos.configVBtn(18, VBtn::LEFT);

    pinInfos.configVBtn(21, VBtn::A);
    pinInfos.configVBtn(34, VBtn::B);
    pinInfos.configVBtn(35, VBtn::LT);
    pinInfos.configVBtn(36, VBtn::RT);

    pinInfos.configVBtn(37, VBtn::X);
    pinInfos.configVBtn(38, VBtn::Y);
    pinInfos.configVBtn(39, VBtn::LB);
    pinInfos.configVBtn(40, VBtn::RB);

    pinInfos.configVBtn(4, VBtn::LS);
    pinInfos.configVBtn(5, VBtn::RS);
    pinInfos.configVBtn(6, VBtn::HOME);
    pinInfos.configVBtn(7, VBtn::A, VBtn::B);
    pinInfos.displayIdx[7] = (int)VBtn::EMPTY;

    pinInfos.configVBtn(2, VBtn::START);
    pinInfos.configVBtn(41, VBtn::BACK);

    #endif

    #ifdef BOARD_DEVKITC

    pinInfos.configVBtn(36, VBtn::A);
    pinInfos.configVBtn(35, VBtn::B);
    pinInfos.configVBtn(6, VBtn::X);
    pinInfos.configVBtn(7, VBtn::Y);

    pinInfos.configVBtn(15, VBtn::LB);
    pinInfos.configVBtn(16, VBtn::RB);
    pinInfos.configVBtn(17, VBtn::LS);
    pinInfos.configVBtn(18, VBtn::RS);

    pinInfos.configVBtn(11, VBtn::UP);
    pinInfos.configVBtn(12, VBtn::DOWN);
    pinInfos.configVBtn(13, VBtn::RIGHT);
    pinInfos.configVBtn(14, VBtn::LEFT);

    pinInfos.configVBtn(1, VBtn::LT);
    pinInfos.configVBtn(2, VBtn::RT);

    pinInfos.configVBtn(3, VBtn::BACK);
    pinInfos.configVBtn(8, VBtn::START);
    pinInfos.configVBtn(9, VBtn::HOME);

    #endif

    pinInfos.autoLayout = false;
}