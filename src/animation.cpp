#include "animation.h"

// ---------------------------------------------------------------------------
// HSV to RGB helper (integer-only, no floating point)
// ---------------------------------------------------------------------------
void hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v,
                uint8_t &r, uint8_t &g, uint8_t &b) {
    if (s == 0) {
        r = g = b = v;
        return;
    }

    uint8_t region    = h / 43;               // 0-5
    uint8_t remainder = (h - region * 43) * 6; // 0-252

    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:  r = v; g = t; b = p; break;
        case 1:  r = q; g = v; b = p; break;
        case 2:  r = p; g = v; b = t; break;
        case 3:  r = p; g = q; b = v; break;
        case 4:  r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
}

// ---------------------------------------------------------------------------
// RainbowCycleAnimation – all LEDs show the same hue, cycling through the
// full spectrum.  Gives the impression of a system powering up.
// ---------------------------------------------------------------------------
RainbowCycleAnimation::RainbowCycleAnimation(uint32_t duration_ms,
                                             uint32_t frame_delay_ms,
                                             uint8_t brightness)
    : duration_ms_(duration_ms), frame_delay_ms_(frame_delay_ms),
      brightness_(brightness), start_time_(0), last_frame_time_(0),
      hue_offset_(0), complete_(false) {}

void RainbowCycleAnimation::start(NeoPixel &strip) {
    start_time_      = to_ms_since_boot(get_absolute_time());
    last_frame_time_ = start_time_;
    hue_offset_      = 0;
    complete_        = false;
}

void RainbowCycleAnimation::update(NeoPixel &strip) {
    if (complete_) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (now - start_time_ >= duration_ms_) {
        complete_ = true;
        return;
    }

    if (now - last_frame_time_ < frame_delay_ms_) return;
    last_frame_time_ = now;

    // Every LED gets the same colour (cycling in unison)
    uint8_t hue = (uint8_t)(hue_offset_ & 0xFF);
    uint8_t r, g, b;
    hsv_to_rgb(hue, 255, brightness_, r, g, b);

    strip.fill(r, g, b);

    hue_offset_ += 3;  // controls rotation speed
}

bool RainbowCycleAnimation::isComplete() const { return complete_; }

// ---------------------------------------------------------------------------
// RainbowChaseAnimation – each LED has a different hue offset so the colours
// appear to travel along the strip.
// ---------------------------------------------------------------------------
RainbowChaseAnimation::RainbowChaseAnimation(uint32_t duration_ms,
                                             uint32_t frame_delay_ms,
                                             uint8_t brightness)
    : duration_ms_(duration_ms), frame_delay_ms_(frame_delay_ms),
      brightness_(brightness), start_time_(0), last_frame_time_(0),
      hue_offset_(0), complete_(false) {}

void RainbowChaseAnimation::start(NeoPixel &strip) {
    start_time_      = to_ms_since_boot(get_absolute_time());
    last_frame_time_ = start_time_;
    hue_offset_      = 0;
    complete_        = false;
}

void RainbowChaseAnimation::update(NeoPixel &strip) {
    if (complete_) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (now - start_time_ >= duration_ms_) {
        complete_ = true;
        return;
    }

    if (now - last_frame_time_ < frame_delay_ms_) return;
    last_frame_time_ = now;

    uint num_pixels = strip.getNumPixels();
    for (uint i = 0; i < num_pixels; i++) {
        uint8_t hue = (uint8_t)((hue_offset_ + i * 256 / num_pixels) & 0xFF);
        uint8_t r, g, b;
        hsv_to_rgb(hue, 255, brightness_, r, g, b);
        strip.setPixelColor(i, r, g, b);
    }

    hue_offset_ += 5;  // faster sweep for chase effect
}

bool RainbowChaseAnimation::isComplete() const { return complete_; }

// ---------------------------------------------------------------------------
// SolidColorAnimation – fill all LEDs with one colour for a fixed time.
// ---------------------------------------------------------------------------
SolidColorAnimation::SolidColorAnimation(uint8_t r, uint8_t g, uint8_t b,
                                         uint32_t duration_ms)
    : r_(r), g_(g), b_(b), duration_ms_(duration_ms),
      start_time_(0), applied_(false), complete_(false) {}

void SolidColorAnimation::start(NeoPixel &strip) {
    start_time_ = to_ms_since_boot(get_absolute_time());
    applied_    = false;
    complete_   = false;
}

void SolidColorAnimation::update(NeoPixel &strip) {
    if (complete_) return;

    if (!applied_) {
        strip.fill(r_, g_, b_);
        applied_ = true;
    }

    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - start_time_ >= duration_ms_) {
        complete_ = true;
    }
}

bool SolidColorAnimation::isComplete() const { return complete_; }

// ---------------------------------------------------------------------------
// FlickerAnimation – rapidly toggles between a colour and black, then holds
// LEDs off for a specified period before completing.
// ---------------------------------------------------------------------------
FlickerAnimation::FlickerAnimation(uint8_t r, uint8_t g, uint8_t b,
                                   uint32_t flicker_duration_ms,
                                   uint32_t off_duration_ms,
                                   uint32_t flicker_interval_ms)
    : r_(r), g_(g), b_(b),
      flicker_duration_ms_(flicker_duration_ms),
      off_duration_ms_(off_duration_ms),
      flicker_interval_ms_(flicker_interval_ms),
      start_time_(0), off_start_time_(0),
      flickering_(true), in_off_phase_(false), complete_(false) {}

void FlickerAnimation::start(NeoPixel &strip) {
    start_time_    = to_ms_since_boot(get_absolute_time());
    flickering_    = true;
    in_off_phase_  = false;
    complete_      = false;
}

void FlickerAnimation::update(NeoPixel &strip) {
    if (complete_) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (flickering_) {
        uint32_t elapsed = now - start_time_;
        if (elapsed >= flicker_duration_ms_) {
            // Transition to dark hold phase
            flickering_   = false;
            in_off_phase_ = true;
            off_start_time_ = now;
            strip.clear();
            return;
        }

        // Rapid on / off toggling
        bool on = ((elapsed / flicker_interval_ms_) % 2) == 0;
        if (on) {
            strip.fill(r_, g_, b_);
        } else {
            strip.clear();
        }
    } else if (in_off_phase_) {
        if (now - off_start_time_ >= off_duration_ms_) {
            complete_ = true;
        }
    }
}

bool FlickerAnimation::isComplete() const { return complete_; }

// ---------------------------------------------------------------------------
// StaticPatternAnimation – sets individual pixel colours and holds forever.
// ---------------------------------------------------------------------------
StaticPatternAnimation::StaticPatternAnimation(const PixelColor *colors,
                                               uint num_pixels)
    : num_pixels_(num_pixels > MAX_PIXELS ? MAX_PIXELS : num_pixels),
      applied_(false) {
    for (uint i = 0; i < num_pixels_; i++) {
        colors_[i] = colors[i];
    }
}

void StaticPatternAnimation::start(NeoPixel &strip) {
    applied_ = false;
}

void StaticPatternAnimation::update(NeoPixel &strip) {
    if (!applied_) {
        for (uint i = 0; i < num_pixels_; i++) {
            strip.setPixelColor(i, colors_[i].r, colors_[i].g, colors_[i].b);
        }
        applied_ = true;
    }
}

bool StaticPatternAnimation::isComplete() const {
    return false;  // static pattern runs indefinitely
}

// ---------------------------------------------------------------------------
// AnimationSequencer
// ---------------------------------------------------------------------------
AnimationSequencer::AnimationSequencer()
    : count_(0), current_(0), started_(false) {
    for (uint i = 0; i < MAX_ANIMATIONS; i++) {
        animations_[i] = nullptr;
    }
}

void AnimationSequencer::addAnimation(Animation *animation) {
    if (count_ < MAX_ANIMATIONS) {
        animations_[count_++] = animation;
    }
}

void AnimationSequencer::start(NeoPixel &strip) {
    current_ = 0;
    started_ = true;
    if (count_ > 0) {
        animations_[0]->start(strip);
    }
}

void AnimationSequencer::update(NeoPixel &strip) {
    if (!started_ || current_ >= count_) return;

    Animation *anim = animations_[current_];
    anim->update(strip);

    if (anim->isComplete()) {
        current_++;
        if (current_ < count_) {
            animations_[current_]->start(strip);
        }
    }
}

bool AnimationSequencer::isComplete() const {
    return started_ && current_ >= count_;
}
