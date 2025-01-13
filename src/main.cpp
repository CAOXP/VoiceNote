#include <Arduino.h>
#include <U8g2lib.h>
#include <RBD_Timer.h>
#include <RBD_Button.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "OLED_page.h"
#include "base64.h"
#include "cJSON.h"
#include "UTF8ToGB2312.h"
#include <ESP_Mail_Client.h>
#include <time.h>
#include "mbedtls/md.h"
#include <ArduinoWebsockets.h>
#include <Base64_Arturo.h>
#include <driver/i2s.h>
#include <vector>

#define EEPROM_SIZE 1500 // 这里定义EEPROM大小
#define EMPTY_MARKER 0xFF
#define key 0
#define ADC 4
#define SMTP_HOST "smtp.qq.com"
#define SMTP_PORT 465
#define LRC 5
#define BCLK 6
#define DIN 7
#define I2S_PORT_1 I2S_NUM_1
#define SAMPLE_RATE 16000
#define CHUNK_SIZE 2048
const char *ssid = "GPT_set";
int upPin = 38;    // 按键所接的GPIO口  d
int downPin = 40;  // 按键所接的GPIO口  b
int enterPin = 39; // 按键所接的GPIO口  c
int countnum;

RBD::Button upbutton(upPin, INPUT_PULLUP);
RBD::Button downbutton(downPin, INPUT_PULLUP);
RBD::Button enterbutton(enterPin, INPUT_PULLUP);

int menu_level = 0;
int position = 1;
int STT_state = 0;
int GPT_state = 0;
String ssid_val;
String password_val;
String XF_APPID_val;
String XF_API_Key_val;
String XF_Secret_Key_val;
String NTP_ADD;
String Ali_API_Key_val;
String question;
String answer = "未包含大模型回答文本，设备断电后回答文本不会保存";
String TTS_text = "这是一个测试";
String answerv;
String backcontent = "";
String backtext = "";
String AUTHOR_EMAIL;
String AUTHOR_PASSWORD;
String minimax_apiKey;
String minimax_group_id;
String emailtext = "";
char myCharPointer;
hw_timer_t *timer = NULL;
const int recordTimeSeconds = 30;
const int adc_data_len = 16000 * recordTimeSeconds;
const int data_json_len = adc_data_len * 2 * 1.4;
uint16_t *adc_data;
char *data_json;
uint8_t adc_start_flag = 0;
uint8_t adc_complete_flag = 0;
uint32_t num = 0;
int TTSsignal = 0;
int STTsignal = 0;
int buttonsignal = 0;
int Sparksignal = 0;
int adc_size;
int timeget = 0;
int textlength;
String stttext = "";
String date = "";
String STT_host = "iat-api.xfyun.cn";
String STT_requestLine = "GET /v2/iat HTTP/1.1";
String STT_url = "ws://iat-api.xfyun.cn/v2/iat";
String TTS_host = "tts-api.xfyun.cn";
String TTS_requestLine = "GET /v2/tts HTTP/1.1";
String TTS_url = "ws://tts-api.xfyun.cn/v2/tts";
String Spark_host = "spark-api.xf-yun.com";
String Spark_requestLine = "GET /v4.0/chat HTTP/1.1";
String Spark_url = "ws://spark-api.xf-yun.com/v4.0/chat";
const char *ntpServer = "ntp.aliyun.com";
bool sttste = 0;
uint recordingSize = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

const char *websockets_server_with_auth;
mbedtls_md_context_t ctx1;
mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

using namespace websockets;
WebsocketsClient STTclient;
WebsocketsClient TTSclient;
WebsocketsClient Sparkclient;

SMTPSession smtp;
ESP_Mail_Session session;
WebServer server(80);

void i2s_setup();
void handleRoot();
void IRAM_ATTR onTimer();
void smtpCallback(SMTP_Status status);
void emailSendHtml(String textcontent);
void smtpSetup();
void postTTS(String texttts);
String xunufeiUrl(String hostPart, String requestLine, String url);
String getGPTAnswer(String inputText);
void clean_ps();
void WIFI_connect();
void ntpconnect();
void showTime();
void webpage_set();
void readEE();
void postSTT();
void postSpark(String inputText);
void postTTS(String inputText);
String EEPROMread(int EEposition);
void EEPROMwrite(String EEname, int EEpoistion);

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  u8g2.begin(); // 初始化OLED显示器
  u8g2.enableUTF8Print();
  upbutton.setDebounceTimeout(50);         // 消除抖动时间是50ms
  downbutton.setDebounceTimeout(50);       // 消除抖动时间是50ms
  enterbutton.setDebounceTimeout(50);      // 消除抖动时间是50ms
  u8g2.setFont(u8g2_font_wqy14_t_gb2312b); // 显示屏字体设置
  pinMode(ADC, ANALOG);
  pinMode(key, INPUT_PULLUP);
  delay(100);
  timer = timerBegin(0, 40, true); // 创建计时器
  timerAlarmWrite(timer, 125, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmEnable(timer);
  timerStop(timer);                                                  // 先暂停
  adc_data = (uint16_t *)ps_malloc(adc_data_len * sizeof(uint16_t)); // 设定录音存储空间
  if (!adc_data)
  {
    Serial.println("Failed to allocate memory for adc_data");
  }

  data_json = (char *)ps_malloc(data_json_len * sizeof(char)); // 根据需要调整存储空间大小
  if (!data_json)
  {
    Serial.println("Failed to allocate memory for data_json");
  }

  L0P1(); // 在屏幕上显示对应文字，详见OLED_page.h。下同
  i2s_setup();
  STTclient.onMessage([&](WebsocketsMessage message) { // 讯飞语音识别返回数据接收处理
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message.data());
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    JsonArray ws = doc["data"]["result"]["ws"];
    for (JsonObject word : ws)
    {
      int bg = word["bg"];
      const char *w = word["cw"][0]["w"];
      stttext += w;
    }
    if (doc["data"]["status"] == 2)
    { // 收到结束标志
      sttste = 1;
      Serial.print("问题：");
      Serial.println(stttext);
      clean_ps();
      STTsignal = 2;
    }
  });

  TTSclient.onMessage([&](WebsocketsMessage message) { // 讯飞语音合成返回数据接收和处理
    DynamicJsonDocument responseJson(51200);
    DeserializationError error = deserializeJson(responseJson, message.data());
    const char *response = responseJson["data"]["audio"].as<String>().c_str();
    int response_len = responseJson["data"]["audio"].as<String>().length();
    Serial.printf("lan: %d  \n", response_len);
    int backstatus = responseJson["data"]["status"].as<int>();
    // 分段获取PCM音频数据并输出到I2S上

    if (backstatus == 1 || backstatus == 0)
    {
      for (int i = 0; i < response_len; i += CHUNK_SIZE)
      {
        int remaining = min(CHUNK_SIZE, response_len - i);                                   // 计算剩余数据长度
        char chunk[CHUNK_SIZE] = {0};                                                        // 创建一个缓冲区来存储读取的数据
        int decoded_length = Base64_Arturo.decode(chunk, (char *)(response + i), remaining); // 从response中解码数据到chunk
        size_t bytes_written = 0;
        if (decoded_length > 10)
        {
          i2s_write(I2S_PORT_1, chunk, decoded_length, &bytes_written, portMAX_DELAY); // 播放音频数据
        }
      }
    }
    if (backstatus == 2)
    {
      for (int i = 0; i < response_len; i += CHUNK_SIZE)
      {
        int remaining = min(CHUNK_SIZE, response_len - i);                                   // 计算剩余数据长度
        char chunk[CHUNK_SIZE] = {0};                                                        // 创建一个缓冲区来存储读取的数据
        int decoded_length = Base64_Arturo.decode(chunk, (char *)(response + i), remaining); // 从response中解码数据到chunk
        size_t bytes_written = 0;
        if (decoded_length > 10)
        {
          i2s_write(I2S_PORT_1, chunk, decoded_length, &bytes_written, portMAX_DELAY);
        }
      }
      double audio_delay = response_len / 64.0;
      Serial.println(audio_delay);
      if (audio_delay > 500)
      {
        delay(500);
      }
      else if (audio_delay < 300)
      {
        delay(audio_delay + 150);
      }
      else
      {
        delay(audio_delay);
      }
      Serial.println("Playing complete.");
      TTSsignal = 2;
      i2s_zero_dma_buffer(I2S_PORT_1);
    }

  });
  Sparkclient.onMessage([&](WebsocketsMessage message) { // 讯飞问答大模型返回数据接收处理
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message.data());
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    const JsonArray textArray = doc["payload"]["choices"]["text"];
    for (const JsonObject &textObj : textArray)
    {
      backcontent = textObj["content"].as<String>();
      backtext += backcontent;
    }
    int textcode = doc["header"]["status"].as<int>();
    if (textcode == 2)
    { // 收到结束标志

      Serial.println(backtext);
      Serial.print("回答结束");
      Sparksignal = 2;
    }

  });
  Serial.println("ready to work");
}
uint32_t time1, time2;
void loop()
{
  if (STTclient.available())
  { // 监听讯飞语音识别服务器
    STTclient.poll();
  }
  if (TTSclient.available())
  { // 监听讯飞语音合成服务器
    TTSclient.poll();
  }

  
  if (downbutton.onPressed())
  { // 如果向下拨键
    position++;
    if (position > 4)
    {
      position = 1;
    }
  }
  if (upbutton.onPressed())
  { // 如果向上拨键
    position--;
    if (position < 1)
    {
      position = 4;
    }
  }
  if (enterbutton.onPressed())
  {
    if (menu_level != 0 && position == 4)
    { // 如果按下拨键
      menu_level--;
      position = 1;
    }
    else
    {
      menu_level++;
      position = 1;
    }
  }

  if (menu_level == 0 && position == 1)
  {
    L0P1(); // 在屏幕上显示对应文字，详见OLED_page.h。下同
  }
  if (menu_level == 0 && position == 2)
  {
    L0P2();
  }
  if (menu_level == 0 && position == 3)
  {
    L0P3();
  }
  if (menu_level == 0 && position == 4)
  {
    L0P4();
  }
  if (menu_level == 1 && L0_position == 1)
  {
    wifi_page1();
    WIFI_connect(); // 连接WIFI
    wifi_page2();
    gettime_page1();
    ntpconnect(); // 获取网络时间戳，讯飞服务器对接必要条件
    int i = 0;
    while (timeget == 0)
    {
      showTime();
      i++;
      if (i > 2)
      {
        WiFi.disconnect();
        WIFI_connect();
        ntpconnect();
        i = 0;
      }
    }
    showTime();
    gettime_page2();

    XF_APPID_val = EEPROMread(118);
    XF_API_Key_val = EEPROMread(200);
    XF_Secret_Key_val = EEPROMread(250);

    Serial.print("讯飞APPID:  ");
    Serial.println(XF_APPID_val);
    Serial.print("讯飞APIKEY:  ");
    Serial.println(XF_API_Key_val);
    Serial.print("讯飞SecretKEY:  ");
    Serial.println(XF_Secret_Key_val);

    delay(100);
    if (STT_state == 1)
    {
      int GPT_num = EEPROM.read(110);
      if (GPT_num == 1)
      {
        GPT_state = 1;
      }
      else if (GPT_num == 2)
      {
        GPT_state = 1;
      }
      else if (GPT_num == 3)
      {
        minimax_group_id = EEPROMread(350);
        minimax_apiKey = "";
        int len_apiKey = EEPROM.read(390) * 10 + EEPROM.read(389);
        for (int i = 0; i < len_apiKey; i++)
        {
          char getchar = EEPROM.read(i + 391);
          minimax_apiKey = minimax_apiKey + String(getchar);
        }
        GPT_state = 1;
      }
      else
      {
        no_GPT_selected_page();
        GPT_state = 0;
        delay(3000);
      }
    }
    Serial.println(STT_state);
    Serial.println(GPT_state);
    if (STT_state == 1 && GPT_state == 1)
    { // 进入“开始提问”
      Serial.println("开始提问");
      while (1)
      {
        question_page();
        if (upbutton.onPressed() || downbutton.onPressed())
        {
          menu_level--;
          break;
        }
        if (enterbutton.onPressed())
        {
          Serial.println("开始录音");
          question = "";
          recording_page();
          adc_start_flag = 1;
          timerStart(timer);
          while (!adc_complete_flag)
          {
            ets_delay_us(10);
          }
          Serial.println("录音结束");
          timerStop(timer);
          adc_complete_flag = 0;
          sendquestion_page();
          STTsignal = 1;
          postSTT();
          while (STTsignal == 1)
          {
            if (STTclient.available())
            {
              STTclient.poll();
            }
          }
          answer = getGPTAnswer(stttext);
          emailtext = answer;
          Serial.println("Answer: " + answer);
          stttext = "";
          answer.replace("*", "");
          textlength = answer.length();
          buttonsignal = 1;
          speech_page();
          if (textlength < 150)
          { // 对大模型回答文本进行处理
            TTSsignal = 1;
            postTTS(answer);
            while (TTSsignal == 1)
            {
              if (TTSclient.available())
              {
                TTSclient.poll();
              }
              if (enterbutton.onPressed())
              {
                TTSsignal = 2;
              }
            }
          }
          else
          {
            answer.replace("，", ",");
            answer.replace("。", ".");
            answer.replace("？", "?");
            answer.replace("：", ":");
            answer.replace("！", "!");
            while (textlength > 150)
            {
              bool foundPos = false;
              int lastPos = -1;
              for (int i = 145; i >= 0; i--)
              {
                char c = answer.charAt(i);
                if (c == ',' || c == '.' || c == ';' || c == ':' || c == '?' || c == '!')
                {
                  lastPos = i;
                  foundPos = true;
                  break;
                }
              }
              if (foundPos)
              {
                String readtextA = answer.substring(0, lastPos + 1);
                TTSsignal = 1;
                postTTS(readtextA);
                while (TTSsignal == 1)
                {
                  if (TTSclient.available())
                  {
                    TTSclient.poll();
                  }
                }
                answer = answer.substring(lastPos + 1);
                Serial.println(answer);
                textlength = answer.length();
              }
              else
              {
                String readtextB = answer.substring(0, 140);
                TTSsignal = 1;
                postTTS(readtextB);
                while (TTSsignal == 1)
                {
                  if (TTSclient.available())
                  {
                    TTSclient.poll();
                  }
                }
                answer = answer.substring(140);
                Serial.println(answer);
                textlength = answer.length();
              }
            }
            postTTS(answer);
            TTSsignal = 1;
            while (TTSsignal == 1)
            {
              if (TTSclient.available())
              {
                TTSclient.poll();
              }
            }
          }

          clean_ps();
          Serial.println("end");
        }
      }
      delay(10);
    }
  }

  if (menu_level == 1 && L0_position == 2 && position == 1)
  {
    L1N1P1();
  }
  if (menu_level == 1 && L0_position == 2 && position == 2)
  {
    L1N1P2();
  }
  if (menu_level == 1 && L0_position == 2 && position == 3)
  {
    L1N1P3();
  }
  if (menu_level == 1 && L0_position == 2 && position == 4)
  {
    L1N1P4();
  }
  if (menu_level == 1 && L0_position == 3 && position == 1)
  {
    L1N2P1();
  }
  if (menu_level == 1 && L0_position == 3 && position == 2)
  {
    L1N2P2();
  }
  if (menu_level == 1 && L0_position == 3 && position == 3)
  {
    L1N2P3();
  }
  if (menu_level == 1 && L0_position == 3 && position == 4)
  {
    L1N2P4();
  }
  if (menu_level == 1 && L0_position == 4)
  { // 进入使用教程页面
    videoQRcode();
    while (1)
    {
      delay(10);
      if (enterbutton.onPressed())
      {
        menu_level--;
        break;
      }
    }
  }
  if (menu_level == 2 && L1_position == 1)
  { // 进入信息输入页面
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawUTF8(0, 15, "在手机端连接WIFI名");
    u8g2.drawUTF8(0, 31, "GPT_set，在网页地");
    u8g2.drawUTF8(0, 47, "址栏输192.168.4.1");
    u8g2.drawUTF8(0, 62, "按键返回，详见教程");
    u8g2.sendBuffer();
    WiFi.disconnect();
    delay(200);
    webpage_set();
    while (1)
    {
      server.handleClient();
      if (enterbutton.onPressed())
      {
        Serial.println("back to menu");
        menu_level--;
        break;
      }
    }
  }
  if (menu_level == 2 && L1_position == 2)
  { // 进入音量设置页面
    int vol_val;
    byte read_val = EEPROM.read(111);
    if (read_val == EMPTY_MARKER)
    {
      vol_val = 5;
    }
    else
    {
      vol_val = EEPROM.read(111);
    }

    while (1)
    {
      if (upbutton.onPressed())
      {
        vol_val++;
        if (vol_val > 9)
        {
          vol_val = 9;
        }
      }
      if (downbutton.onPressed())
      {
        vol_val--;
        if (vol_val < 1)
        {
          vol_val = 1;
        }
      }
      char buffer[16];
      sprintf(buffer, "%d", vol_val);
      u8g2.clearBuffer();
      u8g2.drawUTF8(0, 15, "播报音量设置为");
      u8g2.drawUTF8(0, 31, buffer);
      u8g2.drawUTF8(0, 47, "上下拨动调整音量");
      u8g2.drawUTF8(0, 62, "按下按键确认");
      u8g2.sendBuffer();
      if (enterbutton.onPressed())
      {
        Serial.println("back to menu");
        delay(100);
        EEPROM.write(111, vol_val);
        menu_level--;
        break;
      }
      delay(10);
    }
  }
  if (menu_level == 2 && L1_position == 3)
  { // 进入语速设置页面
    int spd_val;
    byte read_val = EEPROM.read(112);
    if (read_val == EMPTY_MARKER)
    {
      spd_val = 5;
    }
    else
    {
      spd_val = EEPROM.read(112);
    }

    while (1)
    {
      if (upbutton.onPressed())
      {
        spd_val++;
        if (spd_val > 9)
        {
          spd_val = 9;
        }
      }
      if (downbutton.onPressed())
      {
        spd_val--;
        if (spd_val < 1)
        {
          spd_val = 1;
        }
      }
      char buffer[16];
      sprintf(buffer, "%d", spd_val);
      u8g2.clearBuffer();
      u8g2.drawUTF8(0, 15, "播报语速设置为");
      u8g2.drawUTF8(0, 31, buffer);
      u8g2.drawUTF8(0, 47, "上下拨动调整语速");
      u8g2.drawUTF8(0, 62, "按下按键确认");
      u8g2.sendBuffer();
      if (enterbutton.onPressed())
      {
        Serial.println("back to menu");
        delay(100);
        EEPROM.write(112, spd_val);
        menu_level--;
        break;
      }
      delay(10);
    }
  }
  if (menu_level == 2 && L1_position == 4)
  { // 进入当前使用大模型页面
    int GPTnum = EEPROM.read(110);
    Serial.println(GPTnum);
    if (GPTnum == 1)
    {
      u8g2.clearBuffer();
      u8g2.setDrawColor(1);
      u8g2.drawUTF8(0, 15, "当前使用的GPT模型");
      u8g2.drawUTF8(0, 31, "讯飞星火");
      u8g2.sendBuffer();
    }
    else if (GPTnum == 2)
    {
      u8g2.clearBuffer();
      u8g2.setDrawColor(1);
      u8g2.drawUTF8(0, 15, "当前使用的GPT模型");
      u8g2.drawUTF8(0, 31, "阿里通义千问");
      u8g2.sendBuffer();
    }
    else if (GPTnum == 3)
    {
      u8g2.clearBuffer();
      u8g2.setDrawColor(1);
      u8g2.drawUTF8(0, 15, "当前使用的GPT模型");
      u8g2.drawUTF8(0, 31, "miniMax大模型");
      u8g2.sendBuffer();
    }
    else
    {
      no_GPT_selected_page();
    }
    while (1)
    {
      delay(10);
      if ((enterbutton.onPressed()))
      {
        menu_level--;
        break;
      }
    }
  }
  if (menu_level == 2 && L1_position == 5 && position == 1)
  {
    L2N1P1();
  }
  if (menu_level == 2 && L1_position == 5 && position == 2)
  {
    L2N1P2();
  }
  if (menu_level == 2 && L1_position == 5 && position == 3)
  {
    L2N1P3();
  }
  if (menu_level == 2 && L1_position == 5 && position == 4)
  {
    L2N1P4();
  }
  if (menu_level == 2 && L1_position == 6)
  { // 进入发送邮件页面
    byte emaildata = EEPROM.read(60);
    byte passworddata = EEPROM.read(80);
    if (emaildata == EMPTY_MARKER || passworddata == EMPTY_MARKER)
    {
      u8g2.clearBuffer();
      u8g2.setDrawColor(1);
      u8g2.drawUTF8(0, 15, "未输入邮箱信息");
      u8g2.sendBuffer();
      delay(2000);
      menu_level--;
      return;
    }
    if (emailtext == "")
    {
      no_answer_page();
      delay(100);
    }
    else
    {
      u8g2.clearBuffer();
      u8g2.setDrawColor(1);
      u8g2.drawUTF8(0, 15, "回答文本邮件");
      u8g2.drawUTF8(0, 31, "正在发送中");
      u8g2.drawUTF8(0, 47, "请等待约1分钟");
      u8g2.sendBuffer();
      readEE();
      if (WiFi.status() != WL_CONNECTED)
      {
        WIFI_connect();
        u8g2.clearBuffer();
        u8g2.setDrawColor(1);
        u8g2.drawUTF8(0, 15, "回答文本邮件");
        u8g2.drawUTF8(0, 31, "正在发送中");
        u8g2.drawUTF8(0, 47, "请等待约1分钟");
        u8g2.sendBuffer();
      }
      delay(50);
      smtpSetup();
      emailSendHtml(emailtext);
    }
    while (1)
    {
      delay(10);
      if ((enterbutton.onPressed()))
      {
        menu_level--;
        break;
      }
    }
  }
  if (menu_level == 3 && L2_position == 1)
  { // 设置使用的大模型
    EEPROM.write(110, 1);
    EEPROM.commit();
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawUTF8(0, 15, "GPT模型已设置为");
    u8g2.drawUTF8(0, 31, "讯飞星火");
    u8g2.sendBuffer();
    delay(2000);
    menu_level--;
    position = 1;
  }
  if (menu_level == 3 && L2_position == 2)
  { // 设置使用的大模型
    EEPROM.write(110, 2);
    EEPROM.commit();
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawUTF8(0, 15, "GPT模型已设置为");
    u8g2.drawUTF8(0, 31, "阿里通义千问");
    u8g2.sendBuffer();
    delay(2000);
    menu_level--;
    position = 1;
  }
  if (menu_level == 3 && L2_position == 3)
  { // 设置使用的大模型
    EEPROM.write(110, 3);
    EEPROM.commit();
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawUTF8(0, 15, "GPT模型已设置为");
    u8g2.drawUTF8(0, 31, "miniMax大模型");
    u8g2.sendBuffer();
    delay(2000);
    menu_level--;
    position = 1;
  }
}

void handleConnect()
{ // 手机上输入信息并提交后，语音助手接收信息并保存
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  String XF_API_Key = server.arg("XF_API_Key");
  String XF_Secret_Key = server.arg("XF_Secret_Key");
  String XF_APPID = server.arg("XF_APPID");
  String Ali_API_Key = server.arg("Ali_API_Key");
  String ntp_add = server.arg("NTP_ADD");
  String Max_apikey = server.arg("Max_apikey");
  String QQ_email = server.arg("QQ_email");
  String Email_Access_Code = server.arg("Email_Access_Code");
  if (ssid.isEmpty() == false || password.isEmpty() == false || XF_API_Key.isEmpty() == false || XF_Secret_Key.isEmpty() == false || XF_APPID.isEmpty() == false || Ali_API_Key.isEmpty() == false || ntp_add.isEmpty() == false || Max_apikey.isEmpty() == false || QQ_email.isEmpty() == false || Email_Access_Code.isEmpty() == false)
  {
    if (ssid.isEmpty() == false)
    {
      EEPROMwrite(ssid, 0);
    }
    if (password.isEmpty() == false)
    {
      EEPROMwrite(password, 30);
    }
    if (XF_APPID.isEmpty() == false)
    {
      EEPROMwrite(XF_APPID, 118);
    }
    if (Ali_API_Key.isEmpty() == false)
    {
      EEPROMwrite(Ali_API_Key, 300);
    }
    if (ntp_add.isEmpty() == false)
    {
      EEPROMwrite(ntp_add, 170);
    }
    if (Max_apikey.isEmpty() == false)
    {
      Serial.println(Max_apikey);
      // EEPROMwrite(Max_apikey,390);
      int Max_len = Max_apikey.length();
      EEPROM.write(390, Max_len / 10);
      EEPROM.write(389, Max_len % 10);
      EEPROM.commit();
      for (int i = 0; i < Max_len; i++)
      {
        char getchar = Max_apikey.charAt(i);
        EEPROM.write(i + 391, getchar);
        EEPROM.commit();
      }
    }
    if (QQ_email.isEmpty() == false)
    {
      EEPROMwrite(QQ_email, 60);
    }
    if (Email_Access_Code.isEmpty() == false)
    {
      EEPROMwrite(Email_Access_Code, 80);
    }
    if (XF_API_Key.isEmpty() == false)
    {
      EEPROMwrite(XF_API_Key, 200);
    }
    if (XF_Secret_Key.isEmpty() == false)
    {
      EEPROMwrite(XF_Secret_Key, 250);
    }

    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawUTF8(0, 15, "更改已保存，按下");
    u8g2.drawUTF8(0, 31, "按键返回上级菜单");
    u8g2.sendBuffer();
    webpage_set();
  }
  else
  {
    Serial.println("未做更改");
  }
}

void handleRoot()
{ // 手机页面构建代码
  int GPT_num = EEPROM.read(110);
  if (GPT_num == 1)
  {
    String html = "<form action='/connect' method='post'>所有信息只需输入提交一次，关机信息不会丢失<br>如不需更改就不用填写，空白提交:<br><br>WIFI名称<input type='text' name='ssid' value=''><br>WIFI密码<input type='password' name='password' value=''><br><br>讯飞星火APPID<input type='text' name='XF_APPID' value=''><br>讯飞星火Secret Key<input type='text' name='XF_Secret_Key' value=''><br>讯飞星火API Key<input type='text' name='XF_API_Key' value=''><br><br>如需发送回答到邮箱，请输入：<br>QQ邮箱<input type='text' name='QQ_email' value=''><br>注意下面输入邮箱授权码，不是邮箱密码<br>QQ邮箱授权码<input type='password' name='Email_Access_Code' value=''><br>NTP服务器地址，正常不用输入<input type='text' name='NTP_ADD' value=''><br><input type='submit' value='提交' style='padding: 20px 30px;font-size: 30px;'></form>";
    server.send(200, "text/html;charset=UTF-8", "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>body{font-size:24px}label{font-size:26px}</style></head><body>" + html + "</body></html>");
  }
  else if (GPT_num == 2)
  {
    String html = "<form action='/connect' method='post'>所有信息只需输入提交一次，关机信息不会丢失<br>如不需更改就不用填写，空白提交:<br><br>WIFI名称<input type='text' name='ssid' value=''><br>WIFI密码<input type='password' name='password' value=''><br><br>讯飞星火APPID<input type='text' name='XF_APPID' value=''><br>讯飞星火Secret Key<input type='text' name='XF_Secret_Key' value=''><br>讯飞星火API Key<input type='text' name='XF_API_Key' value=''><br><br>通义千问大模型API Key<input type='text' name='Ali_API_Key' value=''><br><br>如需发送回答到邮箱，请输入：<br>QQ邮箱<input type='text' name='QQ_email' value=''><br>注意下面输入邮箱授权码，不是邮箱密码<br>QQ邮箱授权码<input type='password' name='Email_Access_Code' value=''><br>NTP服务器地址，正常不用输入<input type='text' name='NTP_ADD' value=''><br><input type='submit' value='提交' style='padding: 20px 30px;font-size: 30px;'></form>";
    server.send(200, "text/html;charset=UTF-8", "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>body{font-size:24px}label{font-size:26px}</style></head><body>" + html + "</body></html>");
  }
  else if (GPT_num == 3)
  {
    String html = "<form action='/connect' method='post'>所有信息只需输入提交一次，关机信息不会丢失<br>如不需更改就不用填写，空白提交:<br><br>WIFI名称<input type='text' name='ssid' value=''><br>WIFI密码<input type='password' name='password' value=''><br><br>讯飞星火APPID<input type='text' name='XF_APPID' value=''><br>讯飞星火Secret Key<input type='text' name='XF_Secret_Key' value=''><br>讯飞星火API Key<input type='text' name='XF_API_Key' value=''><br>miniMax大模型 apiKey<input type='text' name='Max_apikey' value=''><br><br>如需发送回答到邮箱，请输入：<br>QQ邮箱<input type='text' name='QQ_email' value=''><br>注意下面输入邮箱授权码，不是邮箱密码<br>QQ邮箱授权码<input type='password' name='Email_Access_Code' value=''><br>NTP服务器地址，正常不用输入<input type='text' name='NTP_ADD' value=''><br><input type='submit' value='提交' style='padding: 20px 30px;font-size: 30px;'></form>";
    server.send(200, "text/html;charset=UTF-8", "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>body{font-size:24px}label{font-size:26px}</style></head><body>" + html + "</body></html>");
  }
  else
  {
    no_GPT_selected_page();
    delay(2000);
    menu_level--;
    return;
  }
}

void WIFI_connect()
{                          // 连接WIFI
  WiFi.softAPdisconnect(); // 如果之前在热点模式，则断开热点模式
  int ssid_charlen = EEPROM.read(0);
  // Serial.println(String(ssid_charlen));
  ssid_val = "";
  for (int i = 0; i < ssid_charlen; i++)
  {
    char getchar = EEPROM.read(i + 1);
    // Serial.println(getchar);
    ssid_val = ssid_val + String(getchar);
  }
  Serial.println(ssid_val);
  int password_charlen = EEPROM.read(30);
  // Serial.println(String(password_charlen));
  password_val = "";
  for (int i = 0; i < password_charlen; i++)
  {
    char getchar = EEPROM.read(i + 31);
    // Serial.println(getchar);
    password_val = password_val + String(getchar);
  }
  Serial.println(password_val);
  u8g2.clearBuffer();
  u8g2.drawUTF8(0, 15, "WIFI网络连接中");
  u8g2.sendBuffer();

  WiFi.begin(ssid_val.c_str(), password_val.c_str());
  countnum = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
    countnum = countnum + 1;
    if (countnum > 10)
    {
      u8g2.clearBuffer();
      u8g2.drawUTF8(0, 15, "WIFI网络连接超时");
      u8g2.drawUTF8(0, 31, "请检查WIFI网络设置");
      u8g2.drawUTF8(0, 47, "操作参照使用教程");
      u8g2.sendBuffer();
      break;
    }
  }
  if (countnum < 6)
  {
    Serial.println("连接成功");
    Serial.print("IP地址:");
    Serial.println(WiFi.localIP());
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, ssid_val.c_str());
    u8g2.drawUTF8(0, 31, "WIFI网络连接成功");
    u8g2.sendBuffer();
    delay(100);
  }
}

void webpage_set()
{ // 向手机发送信息输入页面
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.on("/connect", HTTP_POST, handleConnect);
  server.begin();
}

void IRAM_ATTR onTimer()
{ // 录音
  portENTER_CRITICAL_ISR(&timerMux);
  if (adc_start_flag == 1)
  {

    adc_data[num] = analogRead(ADC);
    num++;
    if (num >= adc_data_len)
    {
      adc_size = num * sizeof(uint16_t);
      adc_complete_flag = 1;
      adc_start_flag = 0;
      num = 0;
      Serial.println("timeout");
    }
    if (enterbutton.onPressed())
    {
      adc_size = num * sizeof(uint16_t);
      adc_complete_flag = 1;
      adc_start_flag = 0;
      num = 0;
      Serial.println("button pressed");
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void clean_ps()
{ // 清除缓存
  for (int i = 0; i < adc_data_len; i++)
  {
    adc_data[i] = 0;
  }
}

String getAliAnswer(String inputText)
{ // 调用阿里问答大模型
  Ali_API_Key_val = EEPROMread(300);
  Serial.println(Ali_API_Key_val);
  String apiUrl = "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation";
  HTTPClient http;
  http.setTimeout(40000);
  http.begin(apiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String(Ali_API_Key_val));
  String payload = "{\"model\":\"qwen-turbo\",\"input\":{\"messages\":[{\"role\": \"system\",\"content\": \"你是阿里通义千问生活助手机器人，要求下面的回答严格控制在1000字符以内。\"},{\"role\": \"user\",\"content\": \"" + inputText + "\"}]}}";
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode == 200)
  {
    String response = http.getString();
    http.end();
    Serial.println(response);
    DynamicJsonDocument jsonDoc(1024);
    deserializeJson(jsonDoc, response);
    String outputText = jsonDoc["output"]["text"];
    return outputText;
  }
  else
  {
    http.end();
    Serial.printf("Error %i \n", httpResponseCode);
    get_answer_failed_page();
    delay(3000);
    return "<error>";
  }
}

String getmaxAnswer(String inputText)
{ // 调用miniMax问答大模型
  // Serial.println(minimax_apiKey);
  HTTPClient http;
  http.setTimeout(20000);
  String maxUrl = "https://api.minimax.chat/v1/text/chatcompletion_v2";
  http.begin(maxUrl);
  http.addHeader("Content-Type", "application/json");
  String token_key = String("Bearer ") + minimax_apiKey;
  http.addHeader("Authorization", token_key);
  String payload = "{\"model\":\"abab5.5s-chat\",\"messages\":[{\"role\": \"system\",\"content\": \"你是生活语音助手，要求下面的回答简洁 。\"},{\"role\": \"user\",\"content\": \"" + inputText + "\"}]}";
  int httpResponseCode = http.POST(payload);
  Serial.println("have send");
  if (httpResponseCode == 200)
  {
    String response = http.getString();
    http.end();
    Serial.println(response);
    DynamicJsonDocument jsonDoc(1024);
    deserializeJson(jsonDoc, response);
    String outputText = jsonDoc["choices"][0]["message"]["content"];
    return outputText;
  }
  else
  {
    http.end();
    Serial.printf("Error %i \n", httpResponseCode);
    get_answer_failed_page();
    delay(3000);
    return "<error>";
  }
}
String getSparkAnswer(String inputText)
{ // 监听讯飞星火大模型
  Sparksignal = 1;
  backtext = "";
  postSpark(inputText);
  while (Sparksignal == 1)
  {
    if (Sparkclient.available())
    {
      Sparkclient.poll();
    }
  }
  return backtext;
}
String getGPTAnswer(String inputText)
{
  int GPT_num = EEPROM.read(110);
  // Serial.println(GPT_num);
  String back_answer = "";
  if (GPT_num == 1)
  {
    back_answer = getSparkAnswer(inputText);
    return back_answer;
  }
  else if (GPT_num == 2)
  {
    back_answer = getAliAnswer(inputText);
    return back_answer;
  }
  else if (GPT_num == 3)
  {
    back_answer = getmaxAnswer(inputText);
    return back_answer;
  }
  else
  {
    no_GPT_selected_page();
    delay(2000);
    back_answer = "未选择问答大模型";
    return back_answer;
  }
}
void emailSendHtml(String textcontent)
{ // 发送邮件
  /* 定义smtp message消息类 */
  SMTP_Message message;
  /* 定义邮件消息类的名称，发件人，标题和添加收件人 */
  message.sender.name = "AI语音助手";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "AI语音助手";
  message.addRecipient("myAI", AUTHOR_EMAIL);

  message.text.content = textcontent;
  /* 调用发送邮件函数，失败的话，获取失败信息 */
  if (!MailClient.sendMail(&smtp, &message))
  {
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawUTF8(0, 15, "邮件发送失败");
    u8g2.drawUTF8(0, 31, "确认邮件信息，注意");
    u8g2.drawUTF8(0, 47, "填授权码不是填密码");
    u8g2.drawUTF8(0, 62, "按下按键返回");
    u8g2.sendBuffer();
    Serial.println("发送邮件失败，失败原因是 , " + smtp.errorReason());
  }
}
void smtpSetup()
{ // 初始化smtp
  smtp.callback(smtpCallback);
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";
  if (!smtp.connect(&session))
    return;
}
void smtpCallback(SMTP_Status status) // 获取邮件发送状态
{
  Serial.println(status.info());
  if (status.success())
  {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("邮件发送成功个数: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("邮件发送失败个数: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawUTF8(0, 15, "邮件发送成功");
    u8g2.drawUTF8(0, 47, "按下按键返回");
    u8g2.sendBuffer();
    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* 依次获取发送邮件状态 */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);
      ESP_MAIL_PRINTF("收件人: %s邮件发送状态信息\n", result.recipients);
      ESP_MAIL_PRINTF("状态: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("发送时间: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("邮件标题: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
void readEE()
{ // 读取存储数据
  int charlen = EEPROM.read(60);
  Serial.println(String(charlen));
  AUTHOR_EMAIL = "";
  for (int i = 0; i < charlen; i++)
  {
    char getchar = EEPROM.read(i + 61);
    AUTHOR_EMAIL = AUTHOR_EMAIL + String(getchar);
  }
  Serial.println(AUTHOR_EMAIL);
  delay(100);
  charlen = EEPROM.read(80);
  Serial.println(String(charlen));
  AUTHOR_PASSWORD = "";
  for (int i = 0; i < charlen; i++)
  {
    char getchar = EEPROM.read(i + 81);
    AUTHOR_PASSWORD = AUTHOR_PASSWORD + String(getchar);
  }
  Serial.println(AUTHOR_PASSWORD);
}
void EEPROMwrite(String EEname, int EEpoistion)
{ // 写入存储数据
  Serial.println(EEname);
  Serial.println(EEname.length());
  EEPROM.write(EEpoistion, EEname.length());
  EEPROM.commit();
  for (int i = 0; i < EEname.length(); i++)
  {
    char getchar = EEname.charAt(i);
    EEPROM.write(i + EEpoistion + 1, getchar);
    EEPROM.commit();
  }
}
String EEPROMread(int EEposition)
{ // 读取存储数据
  String EEname = "";
  int EEcharlen = EEPROM.read(EEposition);
  Serial.println(EEcharlen);
  for (int i = 0; i < EEcharlen; i++)
  {
    char getchar = EEPROM.read(i + 1 + EEposition);
    EEname = EEname + String(getchar);
  }
  return EEname;
}

void ntpconnect()
{
  NTP_ADD = EEPROMread(170);
  configTime(0, 0, NTP_ADD.c_str()); // 设置UTC时间
  Serial.println("\nWaiting for time");
  while (!time(nullptr))
  {
    Serial.print(".");
    delay(200);
  }
}
void showTime()
{ // 将时间数据设置为讯飞服务器要求的格式
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  timeget = 1;
  char timeString[30];
  strftime(timeString, sizeof(timeString), "%a, %d %b %Y %H:%M:%S GMT", &timeinfo);
  date = timeString;
  Serial.println(date);
  STT_state = 1;
}
String urlEncode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (unsigned int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == ' ')
    {
      // 替换空格为%20
      encodedString += "%20";
    }
    else if (isalnum(c))
    {
      // 不需要编码的字符
      encodedString += c;
    }
    else
    {
      // 对特殊字符进行编码
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
      {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
      {
        code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  return encodedString;
}
String xunufeiUrl(String hostPart, String requestLine, String url)
{ // 讯飞web服务器握手程序
  showTime();
  const char *SecretKey_cstr = XF_Secret_Key_val.c_str();
  const unsigned char *SecretKey_uchar = reinterpret_cast<const unsigned char *>(SecretKey_cstr);

  String datePart = "date: " + date;
  String signString = "host: " + hostPart + "\n" + datePart + "\n" + requestLine;
  // 使用 mbedtls 计算 HMAC-SHA256
  unsigned char hmacResult[32]; // SHA256 产生的哈希结果长度为 32 字节
  const char *signCStr = signString.c_str();
  mbedtls_md_init(&ctx1);
  mbedtls_md_setup(&ctx1, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_hmac_starts(&ctx1, SecretKey_uchar, strlen(SecretKey_cstr));
  mbedtls_md_hmac_update(&ctx1, (const unsigned char *)signCStr, strlen(signCStr));
  mbedtls_md_hmac_finish(&ctx1, hmacResult);
  mbedtls_md_free(&ctx1);
  String base64Hash = base64::encode(hmacResult, sizeof(hmacResult));
  // Serial.println(base64Hash);
  char authorization_origin[256];
  sprintf(authorization_origin, "api_key=\"%s\", algorithm=\"%s\", headers=\"%s\", signature=\"%s\"",
          XF_API_Key_val.c_str(), "hmac-sha256", "host date request-line", base64Hash.c_str());
  // Serial.println(authorization_origin);
  String authorization = base64::encode((const uint8_t *)authorization_origin, strlen(authorization_origin));
  // Serial.println(authorization);
  String xf_url = url;
  xf_url += "?authorization=" + urlEncode(authorization);
  xf_url += "&date=" + urlEncode(date);
  xf_url += "&host=" + hostPart;
  // Serial.println("websocket url: ");
  return xf_url;
}

void i2s_setup()
{ // I2S初始化
  i2s_config_t i2sOut_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = i2s_bits_per_sample_t(16),
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 1024};
  esp_err_t err = i2s_driver_install(I2S_PORT_1, &i2sOut_config, 0, NULL);
  if (err != ESP_OK)
  {
    Serial.printf("I2S driver install failed (I2S_PORT_1): %d\n", err);
    while (true)
      ;
  }
  const i2s_pin_config_t i2sOut_pin_config = {
      .bck_io_num = BCLK,
      .ws_io_num = LRC,
      .data_out_num = DIN,
      .data_in_num = -1};
  err = i2s_set_pin(I2S_PORT_1, &i2sOut_pin_config);
  if (err != ESP_OK)
  {
    Serial.printf("I2S set pin failed (I2S_PORT_1): %d\n", err);
    while (true)
      ;
  }
}
void postSpark(String textspark)
{ // 调用讯飞星火大模型
  String websockets_server_with_auth = xunufeiUrl(Spark_host, Spark_requestLine, Spark_url);
  Serial.println(websockets_server_with_auth);
  bool connected = Sparkclient.connect(websockets_server_with_auth);
  if (connected)
  {
    Serial.println("Spark Connected!");
  }
  else
  {
    Serial.println("Not Connected!");
  }
  String input = "{";
  input += "\"header\":{\"app_id\":\"" + XF_APPID_val + "\",\"uid\":\"12345\"},";
  // Serial.println(input);
  input += "\"parameter\":{\"chat\":{\"domain\":\"4.0Ultra\",\"temperature\":0.5,\"max_tokens\":1024}},";
  input += "\"payload\":{\"message\":{\"text\":[{\"role\":\"system\",\"content\":\"你是一个语音助手，回复字数控制在200字以内。\"},{\"role\":\"user\",\"content\":\"";
  input += textspark;
  input += "\"}]}}}";
  // Serial.println(input);
  Sparkclient.send(input);
}

void postTTS(String texttts)
{ // 调用讯飞语音合成
  String websockets_server_with_auth = xunufeiUrl(TTS_host, TTS_requestLine, TTS_url);
  Serial.println(websockets_server_with_auth);
  bool connected = TTSclient.connect(websockets_server_with_auth);
  if (connected)
  {
    Serial.println("TTS Connected!");
  }
  else
  {
    Serial.println("Not Connected!");
  }

  int TTSvol;
  int TTSspd;
  byte read_val = EEPROM.read(111);
  if (read_val == EMPTY_MARKER)
  {
    TTSvol = 50;
  }
  else
  {
    TTSvol = EEPROM.read(111) * 10;
  }
  read_val = EEPROM.read(112);
  if (read_val == EMPTY_MARKER)
  {
    TTSspd = 50;
  }
  else
  {
    TTSspd = EEPROM.read(112) * 10;
  }
  String TTStextbase64 = base64::encode(texttts);
  DynamicJsonDocument requestJson(51200);
  requestJson["common"]["app_id"] = XF_APPID_val;
  requestJson["business"]["aue"] = "raw";
  requestJson["business"]["vcn"] = "xiaoyan";
  requestJson["business"]["volume"] = TTSvol;
  requestJson["business"]["speed"] = TTSspd;
  requestJson["business"]["tte"] = "UTF8";
  requestJson["business"]["auf"] = "audio/L16;rate=16000";
  requestJson["data"]["status"] = 2;
  requestJson["data"]["text"] = TTStextbase64;

  String payload;
  serializeJson(requestJson, payload);
  Serial.print("payload: ");
  Serial.println(payload);
  TTSclient.send(payload);
  Serial.println("send a text");
}
void postSTT()
{ // 调用讯飞语音识别
  String websockets_server_with_auth = xunufeiUrl(STT_host, STT_requestLine, STT_url);
  // Serial.println(websockets_server_with_auth);
  bool connected = STTclient.connect(websockets_server_with_auth);
  if (connected)
  {
    Serial.println("STT Connected!");
  }
  else
  {
    Serial.println("Not Connected!");
  }
  int status = 0;
  int dataSize = 1280 * 8;
  int audioDataSize = recordingSize * 2;
  uint lan = adc_size / dataSize;
  uint lan_end = adc_size % dataSize;
  if (lan_end > 0)
  {
    lan++;
  }
  for (int i = 0; i < lan; i++)
  {
    if (i == (lan - 1))
    {
      status = 2;
    }
    if (status == 0)
    {
      String input = "{";
      input += "\"common\":{ \"app_id\":\"" + XF_APPID_val + "\"},";
      input += "\"business\":{\"domain\": \"iat\", \"language\": \"zh_cn\", \"accent\": \"mandarin\", \"vinfo\":1,\"vad_eos\":10000},";
      input += "\"data\":{\"status\": 0, \"format\": \"audio/L16;rate=16000\",\"encoding\": \"raw\",\"audio\":\"";
      String base64audioString = base64::encode((uint8_t *)adc_data, dataSize).c_str();
      // Serial.println(base64audioString);
      input += base64audioString;
      input += "\"}}";
      // Serial.printf("input: %d , status: %d \n", i, status);
      STTclient.send(input);
      status = 1;
    }
    else if (status == 1)
    {
      // Serial.println("status=1");
      String input = "{";
      input += "\"data\":{\"status\": 1, \"format\": \"audio/L16;rate=16000\",\"encoding\": \"raw\",\"audio\":\"";
      String base64audioString = base64::encode((uint8_t *)adc_data + (i * dataSize), dataSize).c_str();
      // Serial.println(base64audioString);
      input += base64audioString;
      input += "\"}}";
      // Serial.printf("input: %d , status: %d \n", i, status);
      STTclient.send(input);
    }
    else if (status == 2)
    {
      // Serial.println("status=2");
      if (lan_end == 0)
      {
        String input = "{";
        input += "\"data\":{\"status\": 2, \"format\": \"audio/L16;rate=16000\",\"encoding\": \"raw\",\"audio\":\"";
        String base64audioString = base64::encode((uint8_t *)adc_data + (i * dataSize), dataSize).c_str();
        input += base64audioString;
        input += "\"}}";
        // Serial.printf("input: %d , status: %d \n", i, status);
        STTclient.send(input);
      }
      if (lan_end > 0)
      {
        String input = "{";
        input += "\"data\":{\"status\": 2, \"format\": \"audio/L16;rate=16000\",\"encoding\": \"raw\",\"audio\":\"";
        String base64audioString = base64::encode((uint8_t *)adc_data + (i * dataSize), lan_end).c_str();
        input += base64audioString;
        input += "\"}}";
        // Serial.printf("input: %d , status: %d \n", i, status);
        STTclient.send(input);
      }
    }
    delay(40);
  }
}