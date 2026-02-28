#include <stdio.h>
#include <stdlib.h>
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

    // ── Random green-eyes configuration ─────────────────────────────
    //  Neon green (Gundam sensor / camera green, ~#1AE605)
    const uint8_t NEON_GREEN_R = 15;
    const uint8_t NEON_GREEN_G = 200;
    const uint8_t NEON_GREEN_B = 5;
    const uint32_t GREEN_EYES_DURATION_MS = 10000;  // 10 s

    // Seed PRNG from hardware timer so every boot is different
    srand(to_ms_since_boot(get_absolute_time()));

    bool   greenEyesActive    = false;
    uint32_t greenEyesStart   = 0;
    // First possible trigger 20-60 s after boot
    uint32_t nextGreenEyesTime = to_ms_since_boot(get_absolute_time())
                                 + 20000 + (rand() % 40000);

    // ── Main loop ───────────────────────────────────────────────────
    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        if (greenEyesActive) {
            // Hold neon green until the duration elapses
            if (now - greenEyesStart >= GREEN_EYES_DURATION_MS) {
                greenEyesActive = false;
                // Restore the stable-state pattern
                stablePattern.start(strip);
                stablePattern.update(strip);
                // Schedule the next random trigger (20-60 s from now)
                nextGreenEyesTime = now + 20000 + (rand() % 40000);
            }
        } else {
            sequencer.update(strip);

            // Only trigger once boot-up is finished (stable pattern running)
            bool inStableState = sequencer.getCurrentIndex()
                                 >= sequencer.getCount() - 1;
            if (inStableState && now >= nextGreenEyesTime) {
                greenEyesActive = true;
                greenEyesStart  = now;
                strip.fill(NEON_GREEN_R, NEON_GREEN_G, NEON_GREEN_B);
            }
        }

        sleep_ms(1);
    }
}
