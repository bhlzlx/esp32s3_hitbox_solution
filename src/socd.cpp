#include "socd.h"

void SOCD::updateInput(mode_t socdMode, bool &neg, bool &pos, input_t& lastInput, socd_result_t& result) {
    constexpr uint8_t socdFlag = socd_result_t::negtive | socd_result_t::positive;
    input_t input(neg, pos);
    if(input.val != lastInput.val) { // 输入没变
        if(input.val != socdFlag) {
            result = (socd_result_t)input.val;
        } else {
            result = socd(lastInput, input, socdMode);
        }
        lastInput = input;
    }
    switch(result) {
        case socd_result_t::natural: {
            neg = pos = false; break;
        }
        case socd_result_t::negtive: {
            neg = true; pos = false; break;
        }
        case socd_result_t::positive: {
            neg = false; pos = true; break;
        }
    }
}

SOCD::socd_result_t SOCD::socd(input_t last, input_t current, mode_t mode) {
    switch(mode) {
        case mode_t::natural: {
            return socd_result_t::natural;
        }
        case mode_t::first_win: {
            if(last.val == 0) {
                return socd_result_t::positive;
            }
            return socd_result_t(current.val & last.val);
        }
        case mode_t::last_win: {
            if(last.val == 0) {
                return socd_result_t::positive;
            }
            return socd_result_t(current.val ^ last.val);
        }
        case mode_t::upp: {
            return socd_result_t::positive;
        }
    }
    return socd_result_t::positive;
}

// SOCD::SOCD()
//     : _data(0)
// {}

void SOCD::reset() {
    _data = 0;
}

void SOCD::queryY(bool&up, bool& down, mode_t socdMode) {
    auto lastY = lastInputY();
    socd_result_t outputY = finalY();
    updateInput(socdMode, down, up, lastY, outputY);
    setLastInputY(lastY);
    setFinalY(outputY);
}

void SOCD::queryX(bool &left, bool &right, mode_t socdMode) {
    if(socdMode == mode_t::upp) {
        socdMode = mode_t::natural;
    }
    auto lastX = lastInputX();
    socd_result_t outputX = finalX();
    updateInput(socdMode, left, right, lastX, outputX);
    setLastInputX(lastX);
    setFinalX(outputX);
}

void SOCD::query(bool& up, bool& down, bool &left, bool &right, mode_t socdMode) {
    queryX(left, right, socdMode);
    queryY(up, down, socdMode);
}

// void queryX(bool& left, bool& right, socd_t socd, socd_mode_t mode) {
//     SOCD obj(socd);
//     obj.queryX(left, right, mode);
// }

// void queryY(bool& up, bool& down, socd_t socd, socd_mode_t mode) {
//     SOCD obj(socd);
//     obj.queryY(up, down, mode);
// }