#pragma once

#include <cstdint>

class SOCD {
public:
    enum class mode_t : uint8_t {
        natural,
        last_win,
        first_win,
        upp,
    };
private:
    uint8_t         _data;

    enum socd_result_t : uint8_t {
        natural = 0,
        negtive = 1, // left, down
        positive = 2, // right, up
    };

    struct input_t {
        uint8_t val;
        input_t(bool n = false, bool p = false)
        {
            val = 0;
            if(n) {
                val |= socd_result_t::negtive;
            }
            if(p) {
                val |= socd_result_t::positive;
            }
        }
    };

    socd_result_t finalX() const {
        return socd_result_t((_data >> 2) & 0x3);
    }
    socd_result_t finalY() const {
        return socd_result_t((_data) & 0x3);
    }

    void setFinalX(socd_result_t x) {
        uint8_t v = x << 2;
        _data &= 0b11110011;
        _data |= v;
    }

    void setFinalY(socd_result_t y) {
        uint8_t v = y;
        _data &= 0b11111100;
        _data |= v;
    }

    input_t lastInputX() const {
        input_t input;
        input.val = (_data >> 6) & 3;
        return input;
    }

    input_t lastInputY() const {
        input_t input;
        input.val = (_data >> 4) & 3;
        return input;
    }

    void setLastInputX(input_t input) {
        uint8_t v = input.val << 6;
        _data &= 0b00111111;
        _data |= v;
    }
    
    void setLastInputY(input_t input) {
        uint8_t v = input.val << 4;
        _data &= 0b11001111;
        _data |= v;
    }

private:
    void updateInput(mode_t socdMode, bool &neg, bool &pos, input_t &lastInput, socd_result_t& result);
    socd_result_t socd(input_t last, input_t current, mode_t mode);
public:
    SOCD(uint8_t d = 0)
        : _data(d)
    {}

    void reset();
    void query(bool&up, bool& down, bool &left, bool &right, mode_t socdMode);
    void queryY(bool&up, bool& down, mode_t socdMode);
    void queryX(bool &left, bool &right, mode_t socdMode);
};

// using socd_t = uint8_t;
// enum class socd_mode_t : uint8_t {
//     natural,
//     last_win,
//     first_win,
//     upp,
// };

// socd_t createSOCD() {
//     return 0;
// }

// void queryX(bool& left, bool& right, socd_t socd, socd_mode_t mode);
// void queryY(bool& up, bool& down, socd_t socd, socd_mode_t mode);
