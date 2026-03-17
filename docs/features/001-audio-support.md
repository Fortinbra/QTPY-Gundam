# Feature 001: Audio Support via I2S Amplifier BFF

**Status: Done**

## Summary

Add audio playback support using the Adafruit I2S Amplifier BFF Add-On. Include a toolchain for converting WAV audio files into C++ source arrays that can be compiled directly into the firmware.

## Hardware

- **Adafruit I2S Amplifier BFF Add-On**
- Pin mapping (QT Py RP2040):
  - DIN (data) → A0 / GPIO 29
  - LRCLK (left/right clock) → A1 / GPIO 28
  - BCLK (bit clock) → A2 / GPIO 27

## Audio Conversion Toolchain

### Goal

Provide a Python script that converts standard WAV files into C++ source files containing the raw audio samples as `const` arrays. These arrays are compiled into the firmware binary and played back over I2S at runtime.

### Pipeline

1. Place source `.wav` files in `assets/audio/`.
2. Run the converter script: `python tools/audio/wav2cpp.py <input.wav> <output_dir>`.
3. The script produces a `.h` / `.cpp` pair with:
   - A `const int16_t` array holding the PCM samples.
   - Metadata constants: sample rate, number of samples, number of channels.
4. Include the generated header in firmware code and link the `.cpp` file.

### Conversion Details

- Input: 16-bit PCM WAV (mono or stereo). Other bit depths are converted to 16-bit.
- Stereo files are mixed down to mono by default (flag to keep stereo).
- Sample rate is preserved as-is (no resampling). Recommended: 16 kHz or 22.05 kHz to keep firmware size manageable.
- Output array name is derived from the input filename (sanitized to a valid C++ identifier).

## Directory Layout

```
assets/
  audio/            # Source WAV files (not compiled, gitignored raw assets)
tools/
  audio/
    wav2cpp.py      # WAV → C++ converter script
    README.md       # Usage instructions
src/
  audio/            # Generated C++ audio data files (committed to repo)
```

## Future Work (Out of Scope for This Feature)

- I2S driver implementation and playback engine (separate feature doc).
- Runtime audio mixing or streaming from flash.
- Compression (ADPCM, IMA) to reduce firmware size.

## Constraints

- Total firmware size is limited by the RP2040's 16 MB flash (QT Py variant). Keep audio clips short.
- 264 KB SRAM — audio buffers must be kept small; stream from flash arrays, don't copy entire clips to RAM.
