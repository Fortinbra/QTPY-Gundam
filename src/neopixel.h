#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include "pico/stdlib.h"
#include "hardware/pio.h"

// NeoPixel driver class for WS2812/WS2812B LEDs
class NeoPixel {
public:
    NeoPixel(uint pin, uint num_pixels, PIO pio = pio0, uint sm = 0);
    ~NeoPixel();
    
    // Set a single pixel color (RGB)
    void setPixelColor(uint pixel, uint8_t r, uint8_t g, uint8_t b);
    
    // Set all pixels to the same color
    void fill(uint8_t r, uint8_t g, uint8_t b);
    
    // Update the LEDs with current buffer
    void show();
    
    // Clear all pixels (set to black)
    void clear();
    
    // Get number of pixels
    uint getNumPixels() const { return num_pixels_; }
    
    // Rainbow animation helper
    void rainbow(uint32_t offset);

private:
    PIO pio_;
    uint sm_;
    uint pin_;
    uint num_pixels_;
    uint offset_;
    
    void putPixel(uint32_t pixel_grb);
    static uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
};

#endif // NEOPIXEL_H
