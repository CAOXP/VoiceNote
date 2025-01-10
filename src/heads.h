#include <wakeup_detect_houguoxiong_inferencing.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>
#include <UrlEncode.h>
#include <base64.hpp>



// 函数声明
static bool microphone_inference_start(uint32_t n_samples);
static bool microphone_inference_record(void);
static int microphone_audio_signal_get_data(size_t offset, size_t length, float* out_ptr);
void playAudio_Zai(void);
void mainChat(void* arg);
String getAccessToken(const char* api_key, const char* secret_key);
String baiduSTT_Send(String access_token, uint8_t* audioData, int audioDataSize);
String baiduErnieBot_Get(String access_token, String prompt);
void baiduTTS_Send(String access_token, String text);
void playAudio(uint8_t* audioData, size_t audioDataSize);
void clearAudio(void);
