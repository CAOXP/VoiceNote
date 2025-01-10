
#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include <driver/i2s.h>


// I2S配置 - MAX98357A
#define I2S_OUT_PORT I2S_NUM_1
#define I2S_OUT_BCLK 15
#define I2S_OUT_LRC 16
#define I2S_OUT_DOUT 7
#define SAMPLE_RATE_PLAY 16000

// I2S配置 - INMP441
#define I2S_IN_PORT I2S_NUM_0
#define I2S_IN_BCLK 4
#define I2S_IN_LRC 5
#define I2S_IN_DIN 6

// 录音配置
#define SAMPLE_RATE_RECORD 16000
#define RECORD_TIME_SECONDS 15
#define BUFFER_SIZE (SAMPLE_RATE_RECORD * RECORD_TIME_SECONDS)


void initRecord(void);
void initAudio(void);
void playAudio(uint8_t* audioData, size_t audioDataSize);
void playAudio_Zai(void);
void clearAudio(void);

#endif
