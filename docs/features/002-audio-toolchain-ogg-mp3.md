# Feature 002: Audio Toolchain — OGG and MP3 Support

**Status: Done**

## Summary

Extend the `wav2cpp.py` audio conversion tool to accept OGG (Vorbis) and MP3 input files in addition to WAV. The tool will decode compressed audio to raw PCM and produce the same C++ source array output.

## Motivation

WAV files are large and inconvenient for storing source audio assets. Most sound effects and audio clips are distributed as MP3 or OGG. Supporting these formats removes the manual step of converting to WAV before running the toolchain.

## Design

### Approach

Add decoding support to `wav2cpp.py` using Python libraries:

- **OGG Vorbis:** Use the `soundfile` library (via libsndfile) or `pydub`.
- **MP3:** Use `pydub` (which wraps FFmpeg/libav) or `soundfile` with appropriate backend.

A single dependency — `pydub` with FFmpeg — covers both formats and keeps the tool simple.

### Changes to `wav2cpp.py`

1. Detect input format by file extension (`.wav`, `.ogg`, `.mp3`).
2. For WAV: use the existing stdlib `wave` module (no new dependency).
3. For OGG/MP3: use `pydub` to decode to raw 16-bit PCM, then feed into the existing sample processing pipeline.
4. All existing flags (`--stereo`, `--name`) continue to work regardless of input format.

### Dependencies

- **`pydub`** — Python package (`pip install pydub`)
- **FFmpeg** — system dependency required by `pydub` for MP3/OGG decoding

Add a `requirements.txt` in `tools/audio/` listing `pydub`.

### Updated Usage

```bash
# WAV (unchanged, still works with stdlib only)
python tools/audio/wav2cpp.py assets/audio/startup.wav

# OGG (requires pydub + FFmpeg)
python tools/audio/wav2cpp.py assets/audio/alert.ogg

# MP3
python tools/audio/wav2cpp.py assets/audio/beep.mp3
```

## Directory Layout (No Changes)

```
assets/audio/       ← source audio files (.wav, .ogg, .mp3)
tools/audio/
    wav2cpp.py      ← updated converter script
    requirements.txt ← pydub dependency
    README.md       ← updated usage instructions
src/audio/          ← generated C++ output
```

## Acceptance Criteria

- [ ] `wav2cpp.py` accepts `.ogg` files and produces correct C++ output.
- [ ] `wav2cpp.py` accepts `.mp3` files and produces correct C++ output.
- [ ] WAV input continues to work without any new dependencies.
- [ ] `requirements.txt` is added to `tools/audio/`.
- [ ] `tools/audio/README.md` is updated with new format support and dependency instructions.
- [ ] Error message is shown if `pydub`/FFmpeg is missing and a non-WAV file is provided.

## Out of Scope

- FLAC, AAC, or other format support (can be added later — `pydub` supports them).
- Resampling (preserve source sample rate as-is).
- Streaming decode for very large files.
