#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <Arduino.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tusb.h"
#include "class/hid/hid_device.h"
#include "device/usbd_pvt.h"


/********* Report ***************/

typedef struct __attribute__((packed, aligned(1))) {
	uint8_t rid;
	uint8_t rsize;
	uint8_t digital_buttons_1;
	uint8_t digital_buttons_2;
	uint8_t lt;
	uint8_t rt;
	int16_t l_x;
	int16_t l_y;
	int16_t r_x;
	int16_t r_y;
	uint8_t reserved_1[6];
} ReportDataXinput;

// extern ReportDataXinput XboxButtonData;      // data to send

class Esp32XInput {
private:
    ReportDataXinput    realtimeReport_;
    ReportDataXinput    lastReport_;
private:
    void _resetRealtimeReport();
    bool _inputChanged();
    void _sendReportData();
public:
    bool init();

    void enterLoop();
    void leaveLoop();

    ReportDataXinput& getRealtimeReport() {
        return realtimeReport_;
    }
};

// void resetReportStruct();
// void sendReportData();