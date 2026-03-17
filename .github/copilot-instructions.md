# Copilot Instructions for QTPY-Gundam

## Project Overview

This is an embedded C++ project for a Gundam model head LED controller. It runs on an **Adafruit QT Py RP2040** microcontroller using the Raspberry Pi Pico SDK (v2.2.0).

## Hardware

- **MCU:** Adafruit QT Py RP2040 (board: `adafruit_qtpy_rp2040`)
- **NeoPixel Driver:** Adafruit NeoPixel Driver BFF — drives WS2812/WS2812B LEDs via PIO on GPIO 26
- **Power:** Adafruit LiPoly Charger BFF Add-On — LiPo battery charging and power management
- **Audio:** Adafruit I2S Amplifier BFF Add-On — I2S digital audio output
- **LEDs:** 4 WS2812 NeoPixels with GRB color order (R and G channels are swapped in software)

## Build System

- CMake (minimum 3.13) with the Pico SDK CMake toolchain
- C11 / C++17
- PIO assembly for WS2812 timing (`src/ws2812.pio`)
- Ninja build via `.pico-sdk` toolchain
- Build command: `ninja -C build`
- Output: `.uf2` firmware file

## Code Structure

- `src/main.cpp` — Entry point, boot-up animation sequence, main loop with random green-eyes effect
- `src/neopixel.h/.cpp` — NeoPixel WS2812 LED driver using RP2040 PIO hardware
- `src/animation.h/.cpp` — Animation framework: base `Animation` class and concrete types (RainbowCycle, RainbowChase, SolidColor, Flicker, StaticPattern), plus `AnimationSequencer`
- `src/ws2812.pio` — PIO assembly program for WS2812 signal timing

## Coding Conventions

- Use the Pico SDK APIs (`pico/stdlib.h`, `hardware/pio.h`, etc.) for all hardware interaction
- Prefer PIO-based I/O for timing-critical peripherals (NeoPixels, I2S)
- Use C++ classes for drivers and animations; inherit from `Animation` base class for new animation types
- Color values use GRB wire order — when setting pixel colors, the first byte is green, the second is red. Account for this when specifying RGB colors in code.
- Keep memory usage low — this is an embedded target with 264 KB SRAM
- Use `to_ms_since_boot(get_absolute_time())` for timing, not blocking delays in the main loop
- Both UART and USB stdio are enabled for debug output

## QT Py RP2040 Analog Pin Mapping

| Label | GPIO |
|-------|------|
| A0    | 29   |
| A1    | 28   |
| A2    | 27   |
| A3    | 26   |

## BFF Add-On Pin Notes

The BFF add-ons stack onto the QT Py via castellated pads. Key pin assignments:
- **NeoPixel data:** A3 / GPIO 26
- **I2S Amplifier BFF:** DIN → A0 / GPIO 29, LRCLK → A1 / GPIO 28, BCLK → A2 / GPIO 27
- **LiPoly Charger BFF:** Managed via onboard charge controller. BatMon (A2 / GPIO 27) is available but unused — it conflicts with the I2S Amplifier BFF's BCLK on the same pin. Cut or leave the BatMon trace disconnected.

## Development Workflow

This project follows a **feature doc driven** workflow. All new features must have a corresponding feature document in the `docs/features/` directory before implementation begins. Do not implement a feature without a feature doc. When asked to add a feature, create or reference the feature doc first, then implement from it.
