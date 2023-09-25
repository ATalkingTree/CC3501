#include "leds.h"

// ws2812 constructor
ws2812::ws2812(PIO pio, int ws2812_pin) {
    this->_pio = pio;
    this->sm = 0;
    uint pio_program_offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, pio_program_offset, ws2812_pin, 800000, false);
}

// ws2812 member functions
void ws2812::set_led(int led, uint8_t red, uint8_t green, uint8_t blue) {
    uint32_t colour = rgb_u32(red, green, blue);
    leds[led] = colour;
}

void ws2812::update_leds() {
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(_pio, sm, leds[i]);
    }
    sleep_ms(500);
}

void ws2812::clear_leds() {
    for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = 0;
    }
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(_pio, sm, leds[i]);
    }
    sleep_ms(100);
}
