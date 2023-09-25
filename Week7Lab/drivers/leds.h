#pragma once

#include <stdio.h>
#include <array>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "WS2812.pio.h"
#include "board.h"

class ws2812 {
public:
    ws2812(PIO, int);
    void set_led(int led, uint8_t red, uint8_t green, uint8_t blue);
    void update_leds();
    void clear_leds();

private:
    int sm;
    PIO _pio;
    std::array<uint32_t, NUM_LEDS> leds = {0};
};


static inline uint32_t rgb_u32(uint8_t red, uint8_t green, uint8_t blue) {
    return ((uint32_t)(red) << 24) | ((uint32_t)(green) << 16) | ((uint32_t)(blue) << 8);
}
