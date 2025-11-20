#include "neopixel.h"
#include "ws2812.pio.h"
#include <stdio.h>

NeoPixel::NeoPixel(uint pin, uint num_pixels, PIO pio, uint sm) 
    : pio_(pio), sm_(sm), pin_(pin), num_pixels_(num_pixels) {
    
    // Load the PIO program
    offset_ = pio_add_program(pio_, &ws2812_program);
    
    // Initialize the WS2812 driver
    ws2812_program_init(pio_, sm_, offset_, pin_, 800000, false);
    
    printf("NeoPixel initialized: %d pixels on GPIO %d\n", num_pixels_, pin_);
}

NeoPixel::~NeoPixel() {
    pio_sm_set_enabled(pio_, sm_, false);
    pio_remove_program(pio_, &ws2812_program, offset_);
}

void NeoPixel::putPixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio_, sm_, pixel_grb << 8u);
}

uint32_t NeoPixel::urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void NeoPixel::setPixelColor(uint pixel, uint8_t r, uint8_t g, uint8_t b) {
    if (pixel < num_pixels_) {
        putPixel(urgb_u32(r, g, b));
    }
}

void NeoPixel::fill(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = urgb_u32(r, g, b);
    for (uint i = 0; i < num_pixels_; i++) {
        putPixel(color);
    }
}

void NeoPixel::show() {
    // Data is automatically sent to LEDs when written to FIFO
    // This method exists for API compatibility
}

void NeoPixel::clear() {
    fill(0, 0, 0);
}

void NeoPixel::rainbow(uint32_t offset) {
    for (uint i = 0; i < num_pixels_; i++) {
        uint32_t hue = (i * 256 / num_pixels_ + offset) & 0xff;
        uint8_t r, g, b;
        
        if (hue < 85) {
            r = hue * 3;
            g = 255 - hue * 3;
            b = 0;
        } else if (hue < 170) {
            hue -= 85;
            r = 255 - hue * 3;
            g = 0;
            b = hue * 3;
        } else {
            hue -= 170;
            r = 0;
            g = hue * 3;
            b = 255 - hue * 3;
        }
        
        putPixel(urgb_u32(r, g, b));
    }
}
