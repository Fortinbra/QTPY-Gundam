#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "neopixel.h"

// Configuration
#define NEOPIXEL_PIN 26  // QT Py RP2040 NeoPixel BFF typically uses GPIO 12
#define NUM_PIXELS 4

int main()
{
    stdio_init_all();
    
    printf("Gundam LED Controller - 4 Pixels\n");
    
    // Initialize NeoPixel driver
    NeoPixel strip(NEOPIXEL_PIN, NUM_PIXELS);
    
    // Yellow color for first two LEDs (25% brightness)
    const uint8_t yellow_r = 64;
    const uint8_t yellow_g = 50;
    const uint8_t yellow_b = 0;
    
    // Red color for last two LEDs (25% brightness, accounting for GRB format)
    const uint8_t red_r = 0;
    const uint8_t red_g = 64;
    const uint8_t red_b = 0;
    
    // Set first two LEDs to yellow
    strip.setPixelColor(0, yellow_r, yellow_g, yellow_b);
    strip.setPixelColor(1, yellow_r, yellow_g, yellow_b);
    
    // Set last two LEDs to red
    strip.setPixelColor(2, red_r, red_g, red_b);
    strip.setPixelColor(3, red_r, red_g, red_b);
    
    while (true) {
        sleep_ms(1000);
    }
}
