#include "AudioManager.h"

bool AudioManager::initialized = false;

void AudioManager::begin() {
    if (!initialized) {
        i2sInit();
        initialized = true;
    }
}

void AudioManager::i2sInit() {
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = Config::SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024
    };

    i2s_pin_config_t pinConfig = {
        .bck_io_num = Config::BCLK_PIN,
        .ws_io_num = Config::LRC_PIN,
        .data_out_num = Config::DIN_PIN,
        .data_in_num = -1
    };

    i2s_driver_install((i2s_port_t)Config::I2S_PORT, &i2sConfig, 0, NULL);
    i2s_set_pin((i2s_port_t)Config::I2S_PORT, &pinConfig);
}

void AudioManager::record(uint16_t* buffer, size_t* length) {
    // Implementation
}

void AudioManager::play(const char* data, size_t length) {
    if (!initialized) return;
    size_t bytesWritten = 0;
    i2s_write((i2s_port_t)Config::I2S_PORT, data, length, &bytesWritten, portMAX_DELAY);
}

void AudioManager::stop() {
    if (!initialized) return;
    i2s_zero_dma_buffer((i2s_port_t)Config::I2S_PORT);
} 