# Feature 003: Play Audio Clip at Steady State

**Status: Done**

## Summary

Play clip_05 (Gundam title theme) once over the I2S Amplifier BFF when the boot-up LED animation sequence reaches the stable/steady-state pattern.

## Trigger

The boot-up animation sequencer runs through 5 phases. When the final phase (stable pattern — two yellow, two red LEDs) begins, clip_05 plays once.

In `main.cpp`, the steady state is reached when:
```cpp
sequencer.getCurrentIndex() >= sequencer.getCount() - 1
```

## Audio Clip

| Clip | Original File | Sample Rate | Flash Size |
|------|--------------|-------------|------------|
| clip_05 | gundam-title-theme | 44100 Hz | ~403.8 KB |

## Implementation

### I2S Driver

Create an I2S audio driver (`src/i2s_audio.h/.cpp`) that:
1. Configures the RP2040 I2S peripheral (PIO-based) with the correct pin mapping:
   - DIN → A0 / GPIO 29
   - LRCLK → A1 / GPIO 28
   - BCLK → A2 / GPIO 27
2. Accepts a pointer to a `const int16_t[]` sample array and its metadata (sample rate, length).
3. Streams samples to the I2S output non-blocking, feeding data from flash in the main loop or via DMA.

### Playback Integration

In `main.cpp`:
1. Initialize the I2S driver at startup.
2. When the animation sequencer enters steady state, trigger playback of `CLIP_05_SAMPLES`.
3. Playback runs once (not looped).
4. The main loop continues updating LEDs while audio plays — audio must not block LED updates.

### Build Changes

- Add `src/i2s_audio.cpp` to `CMakeLists.txt`.
- Add `src/audio/clip_05.cpp` to `CMakeLists.txt`.
- Link `hardware_pio` and `hardware_dma` libraries (PIO already linked; add DMA if using DMA-based streaming).

## Constraints

- Audio playback must be non-blocking — LED animations continue during playback.
- Clip plays exactly once at steady state entry, not on every loop iteration.
- Flash-resident sample array — do not copy to RAM.

## Out of Scope

- Volume control or audio mixing.
- Playing multiple clips simultaneously.
- Looped or repeated playback.
- Other clips (clip_01–04, clip_06) — future feature docs.
