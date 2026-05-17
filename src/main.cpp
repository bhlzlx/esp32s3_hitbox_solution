#include "soc/usb_serial_jtag_reg.h"
#include "esp32_xinput.h"
#include <array>
#include <vector>
#include <type_traits>
#include <Wifi.h>

#include "common_def.h"
#include "pin_mapping.h"
// #include "display/standard_display.h"
#include "display/layout_display.h"
#include "display/input_display.h"
#include "display/serial_display.h"
#include "socd.h"

#include "driver/rtc_io.h"
#include "tusb.h"

#ifdef MY_DEVKIT
constexpr uint8_t wiredModePin = 4;         // USB,BLE启动模式
constexpr uint8_t defHomePin = 1;           // 默认HOME
constexpr uint8_t defStartPin = 2;         // 默认START
constexpr uint8_t sckPin = 14;              // 屏幕SCK
constexpr uint8_t sdaPin = 13;              // 屏幕SDA
constexpr uint8_t powerPin = 5;             // 电量监测
#endif

#ifdef BOARD_NOLOGO
constexpr uint8_t wiredModePin = 6;         // USB,BLE启动模式
constexpr uint8_t defHomePin = 4;           // 默认HOME
constexpr uint8_t defStartPin = 41;         // 默认START
constexpr uint8_t sckPin = 12;              // 屏幕SCK
constexpr uint8_t sdaPin = 11;              // 屏幕SDA
constexpr uint8_t powerPin = 9;             // 电量监测
#endif

#ifdef BOARD_WALNUT
constexpr uint8_t wiredModePin = 8;         // USB,BLE启动模式
constexpr uint8_t defHomePin = 6;           // 默认HOME
constexpr uint8_t defStartPin = 2;         // 默认START
constexpr uint8_t sckPin = 14;              // 屏幕SCK
constexpr uint8_t sdaPin = 13;             // 屏幕SDA
constexpr uint8_t powerPin = 11;             // 电量监测
#endif

#ifdef BOARD_DEVKITC
constexpr uint8_t defHomePin = 9;
constexpr uint8_t defStartPin = 8;
constexpr uint8_t sckPin = 5;
constexpr uint8_t sdaPin = 4;
constexpr uint8_t powerPin = 42;             // 电量监测
constexpr uint8_t wiredModePin = 41;         // USB,BLE启动模式
#endif

pin_infos_t             pinInfos;
loop_data_t             loopData;
general_config_t        generalConfig;

TaskHandle_t            screenTaskHandle = nullptr;


void screenTask(void* ptr);

// TimeHelper tickTime;
TimeHelper batteryTime("battery");
TimeHelper powerSave("power save");
TimeHelper initTime("init time");

WiredMode wiredMode = WiredMode::Wireless;
Esp32XInput xinput;
bool xinputInited = false;
XboxGamepadDevice *gamepad = nullptr;
BleCompositeHID *compositeHID = nullptr;
//
IDisplay* display = nullptr;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2 = nullptr;


RTC_DATA_ATTR uint32_t sleepCount = 0;

int getBatterLevel() {
    auto voltage = 0.0f;
    voltage += analogReadMilliVolts(powerPin);
    voltage /= 1000;
    auto level = (voltage*2 - 3.2f) / (4.2f-3.2f) * 100;
    if(level < 0) {
        level = 0;
    }
    level*=1.17
    ;
    if(level > 100) {
        level = 100;
    }
    return level;
}

void OnVibrateEvent(XboxGamepadOutputReportData data)
{
    if (data.weakMotorMagnitude > 0 || data.strongMotorMagnitude > 0)
    {
        // digitalWrite(ledPin, LOW);
    }
    else
    {
        // digitalWrite(ledPin, HIGH);
    }
    // Serial.println("Vibration event. Weak motor: " + String(data.weakMotorMagnitude) + " Strong motor: " + String(data.strongMotorMagnitude));
}

void setupWirelessController()
{
    // Uncomment one of the following two config types depending on which controller version you want to use
    // The XBox series X controller only works on linux kernels >= 6.5

    // XboxOneSControllerDeviceConfiguration* config = new XboxOneSControllerDeviceConfiguration();
    XboxSeriesXControllerDeviceConfiguration *config = new XboxSeriesXControllerDeviceConfiguration();

    // The composite HID device pretends to be a valid Xbox controller via vendor and product IDs (VID/PID).
    // Platforms like windows/linux need this in order to pick an XInput driver over the generic BLE GATT HID driver.
    BLEHostConfiguration hostConfig = config->getIdealHostConfiguration();
    // Serial.println("Using VID source: " + String(hostConfig.getVidSource(), HEX));
    // Serial.println("Using VID: " + String(hostConfig.getVid(), HEX));
    // Serial.println("Using PID: " + String(hostConfig.getPid(), HEX));
    // Serial.println("Using GUID version: " + String(hostConfig.getGuidVersion(), HEX));
    // Serial.println("Using serial number: " + String(hostConfig.getSerialNumber()));

    // Set up gamepad
    config->setAutoReport(false);
    gamepad = new XboxGamepadDevice(config);

    // Set up vibration event handler
    FunctionSlot<XboxGamepadOutputReportData> vibrationSlot(OnVibrateEvent);
    gamepad->onVibrate.attach(vibrationSlot);

    // Add all child devices to the top-level composite HID device to manage them
    compositeHID = new BleCompositeHID((char const*)u8"久寿川-HITBOX-BLE", "kusugawa", getBatterLevel());
    compositeHID->addDevice(gamepad);

    // Start the composite HID device to broadcast HID reports
    // Serial.println("Starting composite HID device...");
    compositeHID->begin(hostConfig);
    // 
    WiFi.setSleep(WIFI_PS_MIN_MODEM);
}

void setupSD(StandardDisplay* sd) {
    sd->setBatteryLevel(getBatterLevel());
    sd->setWiredMode(wiredMode);
    if(wiredMode == Wireless) {
        sd->sleepAfter(20000);
    }
}

void setup()
{
    // 初始化TinyUSB（如果尚未初始化）
    tusb_init();
    //
    initTime.updateTimepoint();
    // size_t psram_size = esp_spiram_get_size();
    // printf("PSRAM size: %d bytes\n", psram_size);
    // Serial2.begin();
    loopData.reset();

    rtc_gpio_deinit((gpio_num_t)defHomePin);
    // auto xtalHz = getXtalFrequencyMhz();
    // delay(1000);
    pinInfos.reset();
    loopData.reset();

    setCpuFrequencyMhz(80); // 最低功耗

    pinMode(wiredModePin, INPUT_PULLUP);        // wired/wireless indicator
    pinMode(defHomePin, INPUT_PULLUP);          // home
    pinMode(defStartPin, INPUT_PULLUP);         // start
    pinMode(powerPin, ANALOG);                  // power

    bool restoreDefault = false;
    if(!digitalRead(defStartPin)) {
        restoreDefault = true;
    }

    if(!pinInfos.loadPreference() || restoreDefault) {
        configDefault();
        pinInfos.writePreference();
    }
    generalConfig.loadPreference();
    loopData.socdMode = generalConfig.socd;

    for(uint8_t i = 1; i<MaxPinNumber; ++i) {
        if(pinInfos.vbtns[i][0] != VBtn::INVALID) {
            pinMode(i, INPUT_PULLUP);
            auto msg = std::to_string(i) + "pullup\n";
            // Serial.write(msg.c_str());
            // loopData.registedPins[loopData.registedPinCount++] = i;
        }
    }

}

void initController()
{
    u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, sckPin, sdaPin);
    u8g2->begin();

    wiredMode = tud_ready() ? Wired : Wireless;

    if(sleepCount == 0) {
        // home - remap mode
        if(!digitalRead(defHomePin)) {
            display = new InputDisplay(u8g2, &pinInfos);
        // start - reset to default & enter serial mode
        } else {
            auto sd = new StandardDisplay(u8g2, &pinInfos);
            setupSD(sd);
            display = sd;
        }
    } else {
        auto sd = new StandardDisplay(u8g2, &pinInfos);
        setupSD(sd);
        display = sd;
    }

    if (wiredMode == Wireless) {
        setupWirelessController();
    } else {
        xinputInited = xinput.init();
    }

    batteryTime.updateTimepoint();
    powerSave.updateTimepoint();

    xTaskCreatePinnedToCore(
        screenTask,
        "screenTask",
        10000,
        nullptr,
        1,
        &screenTaskHandle,
        0
    );

    pinMode(45, OUTPUT);
    digitalWrite(45, HIGH);
}

void screenTask(void* ptr) {
    // delay(500);
    TimeHelper helper("screenTask");
    for(;;) {
        if(display) {
            auto nextDisp = display->update(&pinInfos);
            if(nextDisp != display) {
                delete display;
                display = nextDisp;
                if(display) {
                    delay(1000);
                    if(display->type() == display_type_t::standard) {
                        StandardDisplay* sd = (StandardDisplay*)display;
                        setupSD(sd);
                    }
                }
            }
        }

        auto homePressed = loopData.getInputState((int)VBtn::HOME);
        auto startPressed = loopData.getInputState((int)VBtn::START);

        if(homePressed && startPressed) {
            if(loopData.getInputState((int)VBtn::DOWN)) {
                loopData.socdMode = SOCD::mode_t::natural;
                generalConfig.socd = loopData.socdMode;
                generalConfig.writePreference();
            } else if(loopData.getInputState((int)VBtn::LEFT)) {
                loopData.socdMode = SOCD::mode_t::last_win;
                generalConfig.socd = loopData.socdMode;
                generalConfig.writePreference();
            } else if(loopData.getInputState((int)VBtn::RIGHT)) {
                loopData.socdMode = SOCD::mode_t::first_win;
                generalConfig.socd = loopData.socdMode;
                generalConfig.writePreference();
            } else if(loopData.getInputState((int)VBtn::UP)) {
                loopData.socdMode = SOCD::mode_t::upp;
                generalConfig.socd = loopData.socdMode;
                generalConfig.writePreference();
            }
        }

        if(startPressed) {
            if(loopData.getInputState((int)VBtn::X)) {
                if(helper.elapsed() > 300) {
                    helper.updateTimepoint();
                    if(display == nullptr) {
                        if(nullptr == u8g2) {
                            u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, sckPin, sdaPin);
                            u8g2->begin();
                        }
                        auto sd = new StandardDisplay(u8g2, &pinInfos);
                        sd->_mode = StandardDisplay::display_mode_t::input;
                        display = sd;
                        setupSD(sd);
                        u8g2->setPowerSave(false);
                    } else {
                        delete display;
                        display = nullptr;
                        u8g2->setPowerSave(true);
                    }
                }
            }
        }
        // if(
        //     loopData.getInputState((int)VBtn::HOME) &&
        //     loopData.getInputState((int)VBtn::START) &&
        //     loopData.getInputState((int)VBtn::A) &&
        //     loopData.getInputState((int)VBtn::B)
        // ) {
        //     esp_restart();
        // }
        delay(1);
    }
}


void wiredLoop()
{
    if(!loopData.inputChanged) {
        return;
    }

    xinput.enterLoop();

    // xinput mode
    auto &report = xinput.getRealtimeReport();
    uint16_t xinputButtons = 0;
    uint8_t lt = 0, rt = 0;
    for(size_t vbtn = 0; vbtn < (size_t)VBtn::VBTN_COUNT; ++vbtn) {
        VBtn btn = (VBtn)vbtn; // 强转
        switch(btn) {
            case VBtn::A:
            case VBtn::B:
            case VBtn::X:
            case VBtn::Y:
            case VBtn::LB:
            case VBtn::RB:
            case VBtn::LS:
            case VBtn::RS:
            case VBtn::HOME:
            case VBtn::START:
            case VBtn::BACK:
            case VBtn::UP:
            case VBtn::DOWN:
            case VBtn::LEFT:
            case VBtn::RIGHT:
            {
                if(loopData.getInputState(vbtn)) {
                    xinputButtons |= (1 << vbtn);
                }
                break;
            }
            case VBtn::LT: {
                if(loopData.getInputState(vbtn)) {
                    lt = 255;
                }
                break;
            }
            case VBtn::RT: {
                if(loopData.getInputState(vbtn)) {
                    rt = 255;
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    report.digital_buttons_1 = xinputButtons >> 8;
    report.digital_buttons_2 = xinputButtons;
    report.lt = lt;
    report.rt = rt;
    xinput.leaveLoop();

    loopData.swapBuffer();
}

void wirelessLoop()
{
    // TimeHelper th("wireless_loop");
    if (compositeHID->isConnected())
    {
        if(loopData.inputChanged) { //|| tickTime.elapsed() > 1500) { // 决定要不要发送数据
            powerSave.updateTimepoint();
            // tickTime.updateTimepoint();
            // update battery level
            if(batteryTime.elapsed() > 60000) { // update battery info
                batteryTime.updateTimepoint();
                auto bl = getBatterLevel();
                compositeHID->setBatteryLevel(bl);
                if(display && display->type() == display_type_t::standard) {
                    auto standard = (StandardDisplay*)display;
                    standard->setBatteryLevel(bl);
                }
            }
            // ++loopData.sendCount1 = th.elapsed();
        } else {
            if(powerSave.elapsed() > 120000) { // 两分钟没有输入就睡眠
                if(!++sleepCount) {             // 
                    sleepCount = 1;
                } else if(sleepCount != 1) { // 初次不睡眠，因为连接会影响
                    if(display) {
                        delete display;
                        display = nullptr;
                    }
                    if(u8g2) {
                        u8g2->setPowerSave(true);
                    }
                    rtc_gpio_init(gpio_num_t(defHomePin));
                    rtc_gpio_set_direction(gpio_num_t(defHomePin), RTC_GPIO_MODE_INPUT_ONLY);
                    rtc_gpio_pullup_en(gpio_num_t(defHomePin));  // Enable pull-up resistor
                    rtc_gpio_pulldown_dis(gpio_num_t(defHomePin));  // Disable pull-down resistor
                    rtc_gpio_hold_en(gpio_num_t(defHomePin));  // Hold the GPIO configuration in deep sleep
                    esp_sleep_enable_ext0_wakeup(gpio_num_t(defHomePin), 0);  //1 = High, 0 = Low
                    esp_deep_sleep_start();
                } else {
                    powerSave.updateTimepoint();
                }
            }
            return;
        }

        int dpadFlags = 0;
        for(size_t vbtn = 0; vbtn < (size_t)VBtn::VBTN_COUNT; ++vbtn) {
            VBtn btn = (VBtn)vbtn; // 强转
            switch(btn) {
                case VBtn::A:
                case VBtn::B:
                case VBtn::X:
                case VBtn::Y:
                case VBtn::LB:
                case VBtn::RB:
                case VBtn::LS:
                case VBtn::RS:
                case VBtn::HOME:
                case VBtn::START:
                case VBtn::BACK:
                {
                    auto xboxBtn = XinputToXBOXButton[vbtn];
                    if(loopData.getInputState(vbtn)) {
                        gamepad->press(xboxBtn);
                    } else {
                        gamepad->release(xboxBtn);
                    }
                    break;
                }
                case VBtn::UP:
                case VBtn::DOWN:
                case VBtn::LEFT:
                case VBtn::RIGHT:
                {
                    if(loopData.getInputState(vbtn)) {
                        dpadFlags |= XinputToXBOXDpad[vbtn];
                    }
                    break;
                }
                case VBtn::LT: {
                    if(loopData.getInputState(vbtn)) {
                        gamepad->setLeftTrigger(XBOX_TRIGGER_MAX);
                    } else {
                        gamepad->setLeftTrigger(0);
                    }
                    break;
                }
                case VBtn::RT: {
                    if(loopData.getInputState(vbtn)) {
                        gamepad->setRightTrigger(XBOX_TRIGGER_MAX);
                    } else {
                        gamepad->setRightTrigger(0);
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        // loopData.sendCount2 = th.elapsed();

        gamepad->pressDPadDirectionFlag((XboxDpadFlags)dpadFlags);
        gamepad->setLeftThumb(0, 0);
        gamepad->setRightThumb(0, 0);
        gamepad->sendGamepadReport();

        loopData.swapBuffer();
    }
    // Serial.write("test");
    // loopData.sendCount3= th.elapsed();
}

enum LoopState {
    idle,
    modeInited,
};

LoopState loopState = idle;

void loop()
{
    switch(loopState) {
        case idle: {
            if(initTime.elapsed() > 1000) {
                initController();
                loopState = modeInited;
            }
            break;
        }
        case modeInited: {
            if(xinputInited || compositeHID) {
                if(xinputInited || compositeHID->isConnected()) {
                    loopData.updateInputState(&pinInfos);
                }
                if (wiredMode == Wired) {
                    wiredLoop();
                    if(batteryTime.elapsed() > 10000) {
                        batteryTime.updateTimepoint();
                        if(display && display->type() == display_type_t::standard) {
                            auto standard = (StandardDisplay*)display;
                            auto bl = getBatterLevel();
                            standard->setBatteryLevel(bl);
                        }
                    }
                } else if(compositeHID->isConnected()) {
                    wirelessLoop();
                }
            }
            break;
        }
    }
    
    delay(4);
}

#ifdef CONFIG_IDF_TARGET_ESP32

extern "C"
{
    void app_main() {
        setup();
        while(true) {
            loop();
        }
    }
}

#endif