#include <stdio.h>
#include "pico/stdlib.h"
#include "neopixel.h"
#include "animation.h"

// Configuration
#define NEOPIXEL_PIN 26  // QT Py RP2040 NeoPixel BFF typically uses GPIO 12
#define NUM_PIXELS 4

int main()
{
    stdio_init_all();

    printf("Gundam LED Controller - 4 Pixels\n");

    // Initialize NeoPixel driver
    NeoPixel strip(NEOPIXEL_PIN, NUM_PIXELS);

    // ── Boot-up animation sequence ──────────────────────────────────

    // Phase 1: Rainbow cycle on all LEDs in unison (~5 s)
    //          Gives the illusion of a massive computer starting up.
    RainbowCycleAnimation rainbowCycle(5000, 20, 64);

    // Phase 2: Rainbow chase across all LEDs (~3 s)
    RainbowChaseAnimation rainbowChase(3000, 30, 64);

    // Phase 3: All LEDs turn red for 5 s
    SolidColorAnimation solidRed(0, 64, 0, 5000);

    // Phase 4: Flicker effect (~1 s), then LEDs off for 1 s
    FlickerAnimation flicker(0, 64, 0, 1000, 1000, 80);

    // Phase 5: Stable state – two yellow, two red
    StaticPatternAnimation::PixelColor stableColors[NUM_PIXELS] = {
        {64, 50, 0},  // LED 0: Yellow
        {64, 50, 0},  // LED 1: Yellow
        {0,  64, 0},  // LED 2: Red
        {0,  64, 0},  // LED 3: Red
    };
    StaticPatternAnimation stablePattern(stableColors, NUM_PIXELS);

    // Assemble and start the sequence
    AnimationSequencer sequencer;
    sequencer.addAnimation(&rainbowCycle);
    sequencer.addAnimation(&rainbowChase);
    sequencer.addAnimation(&solidRed);
    sequencer.addAnimation(&flicker);
    sequencer.addAnimation(&stablePattern);

    sequencer.start(strip);

    // ── Main loop ───────────────────────────────────────────────────
    while (true) {
        sequencer.update(strip);
        sleep_ms(1);
    }
}
