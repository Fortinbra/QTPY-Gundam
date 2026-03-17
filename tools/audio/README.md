# Audio Conversion Toolchain

Converts audio files into C++ source arrays for embedding in RP2040 firmware.

## Supported Formats

| Format | Extension | Dependencies |
|--------|-----------|-------------|
| WAV    | `.wav`    | None (Python stdlib) |
| OGG    | `.ogg`    | `pydub` + FFmpeg |
| MP3    | `.mp3`    | `pydub` + FFmpeg |

## Requirements

- Python 3

For OGG/MP3 support:

```bash
pip install -r tools/audio/requirements.txt
```

[FFmpeg](https://ffmpeg.org/) must also be installed and available on your PATH.

> **Python 3.13+:** The `audioop` module was removed. Install `audioop-lts` (included in `requirements.txt`) to restore compatibility with `pydub`.

## Usage

```bash
python tools/audio/wav2cpp.py <input_file> [output_dir]
```

Accepts `.wav`, `.ogg`, and `.mp3` files. By default, output goes to `src/audio/`. The script generates a `.h` and `.cpp` pair.

### Options

| Flag | Description |
|------|-------------|
| `--stereo` | Keep stereo channels instead of mixing to mono |
| `--name NAME` | Override the C++ identifier (default: derived from filename) |

### Examples

```bash
# Convert startup.wav → src/audio/startup.h + src/audio/startup.cpp
python tools/audio/wav2cpp.py assets/audio/startup.wav

# Convert an OGG file
python tools/audio/wav2cpp.py assets/audio/alert.ogg

# Convert an MP3 file
python tools/audio/wav2cpp.py assets/audio/beep.mp3

# Convert to a custom output directory
python tools/audio/wav2cpp.py assets/audio/beep.wav src/sounds

# Keep stereo and specify a custom name
python tools/audio/wav2cpp.py assets/audio/alert.ogg --stereo --name ALERT_SOUND
```

## Output Format

The generated files contain:

- `const int16_t NAME_SAMPLES[]` — the raw PCM sample data
- `NAME_SAMPLE_RATE` — sample rate in Hz
- `NAME_NUM_SAMPLES` — total number of samples
- `NAME_NUM_CHANNELS` — 1 (mono) or 2 (stereo)

## Tips

- Use **16 kHz or 22.05 kHz mono** WAV files to keep firmware size small.
- 1 second of 16 kHz mono audio ≈ 32 KB of flash.
- The RP2040 QT Py has 8 MB of flash — keep total audio well under that.

## Audio Clip Mapping

| Clip | Original File | Format | Sample Rate | Flash Size |
|------|--------------|--------|-------------|------------|
| clip_01 | Animage, Gundam Beam Rifle (Ep. 2) | OGG | 96000 Hz | ~410.5 KB |
| clip_02 | Animage, Radar Sensor (Ep. 12) | OGG | 96000 Hz | ~789.2 KB |
| clip_03 | gundam-manuever | MP3 | 44100 Hz | ~112.6 KB |
| clip_04 | gundam-newtype-flash-sound-effect | MP3 | 44100 Hz | ~238.5 KB |
| clip_05 | gundam-title-theme | MP3 | 44100 Hz | ~403.8 KB |
| clip_06 | mech-manuever | MP3 | 48000 Hz | ~117.0 KB |

**Total estimated flash usage: ~2,071.6 KB (~2.0 MB)**
