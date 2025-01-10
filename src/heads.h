#ifndef HEADS_H
#define HEADS_H

#include <wakeup_detect_houguoxiong_inferencing.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <UrlEncode.h>
#include <base64.hpp>
#include <driver/i2s.h>
#include "audio.h"



void mainChat(void* arg);
String getAccessToken(const char* api_key, const char* secret_key);
String baiduSTT_Send(String access_token, uint8_t* audioData, int audioDataSize);
String baiduErnieBot_Get(String access_token, String prompt);
void baiduTTS_Send(String access_token, String text);

#endif
