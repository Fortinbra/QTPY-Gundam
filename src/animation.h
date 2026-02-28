#ifndef ANIMATION_H
#define ANIMATION_H

#include "neopixel.h"
#include "pico/stdlib.h"

// HSV to RGB conversion helper
// h: 0-255 (hue), s: 0-255 (saturation), v: 0-255 (value/brightness)
void hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t &r, uint8_t &g, uint8_t &b);

// ---------------------------------------------------------------------------
// Base class for all animations.
// Subclass this to add new animation types in the future.
// ---------------------------------------------------------------------------
class Animation {
public:
    virtual ~Animation() = default;

    // Called once when the animation begins
    virtual void start(NeoPixel &strip) = 0;

    // Called on every iteration of the main loop to advance the animation
    virtual void update(NeoPixel &strip) = 0;

    // Returns true when the animation has finished its work
    virtual bool isComplete() const = 0;
};

// ---------------------------------------------------------------------------
// Concrete animation types
// ---------------------------------------------------------------------------

// All LEDs cycle through the full rainbow in unison
class RainbowCycleAnimation : public Animation {
public:
    RainbowCycleAnimation(uint32_t duration_ms,
                          uint32_t frame_delay_ms = 20,
                          uint8_t brightness = 64);
    void start(NeoPixel &strip) override;
    void update(NeoPixel &strip) override;
    bool isComplete() const override;

private:
    uint32_t duration_ms_;
    uint32_t frame_delay_ms_;
    uint8_t brightness_;
    uint32_t start_time_;
    uint32_t last_frame_time_;
    uint32_t hue_offset_;
    bool complete_;
};

// Rainbow chase: each LED shows a different hue, creating a traveling wave
class RainbowChaseAnimation : public Animation {
public:
    RainbowChaseAnimation(uint32_t duration_ms,
                          uint32_t frame_delay_ms = 30,
                          uint8_t brightness = 64);
    void start(NeoPixel &strip) override;
    void update(NeoPixel &strip) override;
    bool isComplete() const override;

private:
    uint32_t duration_ms_;
    uint32_t frame_delay_ms_;
    uint8_t brightness_;
    uint32_t start_time_;
    uint32_t last_frame_time_;
    uint32_t hue_offset_;
    bool complete_;
};

// All LEDs set to a single solid color for a fixed duration
class SolidColorAnimation : public Animation {
public:
    SolidColorAnimation(uint8_t r, uint8_t g, uint8_t b, uint32_t duration_ms);
    void start(NeoPixel &strip) override;
    void update(NeoPixel &strip) override;
    bool isComplete() const override;

private:
    uint8_t r_, g_, b_;
    uint32_t duration_ms_;
    uint32_t start_time_;
    bool applied_;
    bool complete_;
};

// Flicker effect: rapidly toggles between a color and off, then stays dark
class FlickerAnimation : public Animation {
public:
    FlickerAnimation(uint8_t r, uint8_t g, uint8_t b,
                     uint32_t flicker_duration_ms,
                     uint32_t off_duration_ms,
                     uint32_t flicker_interval_ms = 80);
    void start(NeoPixel &strip) override;
    void update(NeoPixel &strip) override;
    bool isComplete() const override;

private:
    uint8_t r_, g_, b_;
    uint32_t flicker_duration_ms_;
    uint32_t off_duration_ms_;
    uint32_t flicker_interval_ms_;
    uint32_t start_time_;
    uint32_t off_start_time_;
    bool flickering_;
    bool in_off_phase_;
    bool complete_;
};

// Set individual pixel colors and hold indefinitely (final steady state)
class StaticPatternAnimation : public Animation {
public:
    struct PixelColor {
        uint8_t r, g, b;
    };

    StaticPatternAnimation(const PixelColor *colors, uint num_pixels);
    void start(NeoPixel &strip) override;
    void update(NeoPixel &strip) override;
    bool isComplete() const override;  // Always false – runs forever

private:
    static const uint MAX_PIXELS = 8;
    PixelColor colors_[MAX_PIXELS];
    uint num_pixels_;
    bool applied_;
};

// ---------------------------------------------------------------------------
// Animation sequencer – runs a list of animations in order
// ---------------------------------------------------------------------------
class AnimationSequencer {
public:
    static const uint MAX_ANIMATIONS = 16;

    AnimationSequencer();

    // Append an animation to the sequence (caller retains ownership)
    void addAnimation(Animation *animation);

    // Begin running the sequence from the first animation
    void start(NeoPixel &strip);

    // Advance the current animation; moves to next when complete
    void update(NeoPixel &strip);

    // True when every animation in the sequence has completed
    bool isComplete() const;

    // Index of the currently-running animation
    uint getCurrentIndex() const { return current_; }

    // Total number of animations in the sequence
    uint getCount() const { return count_; }

private:
    Animation *animations_[MAX_ANIMATIONS];
    uint count_;
    uint current_;
    bool started_;
};

#endif // ANIMATION_H
