#include <stdio.h>
#include "pico/stdlib.h"
#include "drivers/leds.h"
#include "drivers/board.h"

int main() {
    stdio_init_all();
    ws2812 leds(pio0, LED_PIN);

    for (;;) {
        leds.set_led(2, 255, 0, 0);
        leds.set_led(5, 0, 255, 0);
        leds.set_led(11, 255, 255, 255);
        leds.update_leds();

        leds.set_led(2, 0, 0, 255);
        leds.set_led(5, 255, 0, 255);
        leds.set_led(11, 255, 255, 0);
        leds.update_leds();

        leds.clear_leds();
        sleep_ms(500);
    }
    return 0;
}
