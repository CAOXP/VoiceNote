#pragma once

// 配置参数
struct Config {
    static const int EEPROM_SIZE = 1500;
    static const int EMPTY_MARKER = 0xFF;
    static const int SAMPLE_RATE = 16000;
    static const int CHUNK_SIZE = 2048;
    
    // GPIO pins
    static const int KEY_PIN = 0;
    static const int ADC_PIN = 4; 
    static const int LRC_PIN = 5;
    static const int BCLK_PIN = 6;
    static const int DIN_PIN = 7;
    static const int UP_PIN = 38;
    static const int DOWN_PIN = 40;
    static const int ENTER_PIN = 39;

    // I2S config
    static const int I2S_PORT = 1;
    
    // SMTP config
    static constexpr const char* SMTP_HOST = "smtp.qq.com";
    static const int SMTP_PORT = 465;
    
    // Recording config
    static const int RECORD_TIME_SECONDS = 30;
    static const int ADC_DATA_LEN = 16000 * RECORD_TIME_SECONDS;
    static const int DATA_JSON_LEN = ADC_DATA_LEN * 2 * 1.4;
}; 