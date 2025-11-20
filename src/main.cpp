#include <stdio.h>
#include "pico/stdlib.h"
#include "neopixel.h"

// Configuration
#define NEOPIXEL_PIN 12  // QT Py RP2040 NeoPixel BFF typically uses GPIO 12
#define NUM_PIXELS 4

int main()
{
    stdio_init_all();
    
    printf("QT Py RP2040 NeoPixel Demo - 4 Pixels\n");
    
    // Initialize NeoPixel driver
    NeoPixel strip(NEOPIXEL_PIN, NUM_PIXELS);
    
    uint32_t offset = 0;
    
    while (true) {
        strip.rainbow(offset);
        offset = (offset + 1) & 0xff;
        sleep_ms(20);
    }
}
