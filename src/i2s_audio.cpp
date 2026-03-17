#include "i2s_audio.h"
#include "i2s_out.pio.h"
#include <stdio.h>
#include <string.h>

I2SAudio *I2SAudio::instance_ = nullptr;

I2SAudio::I2SAudio(uint data_pin, uint bclk_pin, uint lrclk_pin,
                   PIO pio, uint sm)
    : pio_(pio), sm_(sm),
      data_pin_(data_pin), bclk_pin_(bclk_pin), lrclk_pin_(lrclk_pin),
      pio_offset_(0), dma_channel_(-1), playing_(false),
      next_is_a_(true),
      src_samples_(nullptr), src_num_samples_(0), src_pos_(0) {

    // Load PIO program
    pio_offset_ = pio_add_program(pio_, &i2s_out_program);

    // Claim a DMA channel
    dma_channel_ = dma_claim_unused_channel(true);

    // Set up DMA IRQ handler
    instance_ = this;
    irq_set_exclusive_handler(DMA_IRQ_0, dmaIrqHandler);
    irq_set_enabled(DMA_IRQ_0, true);

    printf("I2S Audio initialized: DIN=GPIO%d, BCLK=GPIO%d, LRCLK=GPIO%d\n",
           data_pin_, bclk_pin_, lrclk_pin_);
}

I2SAudio::~I2SAudio() {
    stop();
    irq_set_enabled(DMA_IRQ_0, false);
    pio_sm_set_enabled(pio_, sm_, false);
    pio_remove_program(pio_, &i2s_out_program, pio_offset_);
    if (dma_channel_ >= 0) {
        dma_channel_unclaim(dma_channel_);
    }
    if (instance_ == this) instance_ = nullptr;
}

void I2SAudio::initPio(uint32_t sample_rate) {
    pio_sm_set_enabled(pio_, sm_, false);
    pio_sm_clear_fifos(pio_, sm_);

    i2s_out_program_init(pio_, sm_, pio_offset_,
                         data_pin_, bclk_pin_, lrclk_pin_, sample_rate);
}

uint32_t I2SAudio::fillBuffer(uint32_t *buf) {
    uint32_t remaining = src_num_samples_ - src_pos_;
    uint32_t count = remaining < BUF_SAMPLES ? remaining : BUF_SAMPLES;

    for (uint32_t i = 0; i < count; i++) {
        uint16_t s = (uint16_t)src_samples_[src_pos_++];
        buf[i] = ((uint32_t)s << 16) | (uint32_t)s;
    }

    // Zero-fill remainder for the last partial buffer
    if (count < BUF_SAMPLES) {
        memset(&buf[count], 0, (BUF_SAMPLES - count) * sizeof(uint32_t));
    }

    return count;
}

void I2SAudio::dmaIrqHandler() {
    if (!instance_) return;

    dma_hw->ints0 = 1u << instance_->dma_channel_;

    if (!instance_->playing_ || instance_->src_pos_ >= instance_->src_num_samples_) {
        instance_->playing_ = false;
        return;
    }

    // Fill the next buffer and start a new DMA transfer
    uint32_t *buf = instance_->next_is_a_ ? instance_->buf_a_ : instance_->buf_b_;
    instance_->next_is_a_ = !instance_->next_is_a_;
    uint32_t count = instance_->fillBuffer(buf);

    if (count > 0) {
        dma_channel_transfer_from_buffer_now(instance_->dma_channel_, buf, BUF_SAMPLES);
    } else {
        instance_->playing_ = false;
    }
}

void I2SAudio::play(const int16_t *samples, uint32_t num_samples,
                    uint32_t sample_rate) {
    stop();

    src_samples_ = samples;
    src_num_samples_ = num_samples;
    src_pos_ = 0;
    next_is_a_ = true;

    // Configure PIO for this sample rate
    initPio(sample_rate);

    // Configure DMA to feed the PIO TX FIFO
    dma_channel_config cfg = dma_channel_get_default_config(dma_channel_);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_32);
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);
    channel_config_set_dreq(&cfg, pio_get_dreq(pio_, sm_, true));

    dma_channel_configure(
        dma_channel_,
        &cfg,
        &pio_->txf[sm_],     // write to PIO TX FIFO
        NULL,                 // read address set per transfer
        BUF_SAMPLES,          // transfer count
        false                 // don't start yet
    );

    // Enable DMA completion interrupt for this channel
    dma_channel_set_irq0_enabled(dma_channel_, true);

    // Fill the first buffer and start streaming
    uint32_t *buf = buf_a_;
    next_is_a_ = false;
    fillBuffer(buf);

    playing_ = true;
    dma_channel_transfer_from_buffer_now(dma_channel_, buf, BUF_SAMPLES);

    printf("I2S: playing %lu samples at %lu Hz (streaming)\n",
           (unsigned long)num_samples, (unsigned long)sample_rate);
}

bool I2SAudio::isPlaying() const {
    return playing_;
}

void I2SAudio::stop() {
    if (playing_) {
        playing_ = false;
        dma_channel_set_irq0_enabled(dma_channel_, false);
        dma_channel_abort(dma_channel_);
        pio_sm_set_enabled(pio_, sm_, false);
        pio_sm_clear_fifos(pio_, sm_);
    }
}
