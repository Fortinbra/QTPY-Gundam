#ifndef I2S_AUDIO_H
#define I2S_AUDIO_H

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

class I2SAudio {
public:
    // bclk_pin and lrclk_pin must be consecutive GPIOs (bclk, then bclk+1 = lrclk)
    I2SAudio(uint data_pin, uint bclk_pin, uint lrclk_pin,
             PIO pio = pio1, uint sm = 0);
    ~I2SAudio();

    // Start playing a mono 16-bit PCM sample array at the given sample rate.
    // Non-blocking — uses DMA to stream data from flash in small chunks.
    void play(const int16_t *samples, uint32_t num_samples, uint32_t sample_rate);

    // Returns true while audio is still playing
    bool isPlaying() const;

    // Stop playback immediately
    void stop();

private:
    static constexpr uint32_t BUF_SAMPLES = 256;

    PIO pio_;
    uint sm_;
    uint data_pin_;
    uint bclk_pin_;
    uint lrclk_pin_;
    uint pio_offset_;

    int dma_channel_;
    volatile bool playing_;

    // Double buffers for DMA streaming (mono→stereo, 32-bit per frame)
    uint32_t buf_a_[BUF_SAMPLES];
    uint32_t buf_b_[BUF_SAMPLES];
    bool next_is_a_;

    // Source audio tracking (flash-resident)
    const int16_t *src_samples_;
    uint32_t src_num_samples_;
    volatile uint32_t src_pos_;

    void initPio(uint32_t sample_rate);
    uint32_t fillBuffer(uint32_t *buf);

    static I2SAudio *instance_;
    static void dmaIrqHandler();
};

#endif // I2S_AUDIO_H
