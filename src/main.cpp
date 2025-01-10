#include "heads.h"


// 音频缓冲区结构体
typedef struct
{
  int16_t *buffer;
  uint8_t buf_ready;
  uint32_t buf_count;
  uint32_t n_samples;
} inference_t;


// 唤醒词阈值配置 - 阈值越大，要求识别的唤醒词更精准
#define PRED_VALUE_THRESHOLD 0.8

// WiFi配置
const char *ssid = "";
const char *password = "";

// 百度API配置
const char *baidu_api_key = "";
const char *baidu_secret_key = "";

// 百度千帆大模型配置
char *qianfan_api_key = "";
char *qianfan_secret_key = "";

// 初始化WiFi连接
void initWiFi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}
// 全局变量
static inference_t inference;
static const uint32_t sample_buffer_size = 2048;
static signed short sampleBuffer[sample_buffer_size];
static bool debug_nn = false; // 设置为true可以查看从原始信号生成的特征等信息
static bool record_status = true;

void setup()
{
  // 初始化串口通信
  Serial.begin(115200);

  // 初始化LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED关闭

  // 初始化WiFi
  initWiFi();

  // 初始化音频输出
  initAudio();

  // 初始化音频输入
  initRecord();

  // 启动对话主流程
  xTaskCreate(mainChat, "mainChat", 1024 * 32, NULL, 10, NULL);

  // 打印推理设置摘要(来自model_metadata.h)
  ei_printf("Inferencing settings:\n");
  ei_printf("\tInterval: ");
  ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
  ei_printf(" ms.\n");
  ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
  ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

  ei_printf("\nStarting continious inference in 2 seconds...\n");
  ei_sleep(2000);

  if (microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT) == false)
  {
    ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
    return;
  }

  ei_printf("Recording...\n");
}

/**
 * @brief Arduino主循环函数,运行推理循环
 */
void loop()
{
  bool m = microphone_inference_record();
  if (!m)
  {
    ei_printf("ERR: Failed to record audio...\n");
    return;
  }

  signal_t signal;
  signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
  signal.get_data = &microphone_audio_signal_get_data;
  ei_impulse_result_t result = {0};

  EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
  if (r != EI_IMPULSE_OK)
  {
    ei_printf("ERR: Failed to run classifier (%d)\n", r);
    return;
  }

  int pred_index = -1;                     // 初始化预测索引
  float pred_value = PRED_VALUE_THRESHOLD; // 初始化预测值

  // 打印预测结果
  ei_printf("Predictions ");
  ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
  ei_printf(": \n");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
  {
    ei_printf("    %s: ", result.classification[ix].label);
    ei_printf_float(result.classification[ix].value);
    ei_printf("\n");

    // 检查唤醒词 - 唤醒词在第一位时判断classification[0]
    // 如果唤醒词在第2位则判断classification[1],第3位则判断classification[2]
    if (result.classification[0].value > pred_value)
    {
      pred_index = 0;
    }
  }
  // 显示推理结果
  if (pred_index == 0)
  {
    digitalWrite(LED_BUILTIN, HIGH); // LED开启

    Serial.println("playAudio_Zai");

    playAudio_Zai();

    record_status = false;
  }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("    anomaly score: ");
  ei_printf_float(result.anomaly);
  ei_printf("\n");
#endif
}

static void audio_inference_callback(uint32_t n_bytes)
{
  for (int i = 0; i < n_bytes >> 1; i++)
  {
    inference.buffer[inference.buf_count++] = sampleBuffer[i];

    if (inference.buf_count >= inference.n_samples)
    {
      inference.buf_count = 0;
      inference.buf_ready = 1;
    }
  }
}

static void capture_samples(void *arg)
{

  const int32_t i2s_bytes_to_read = (uint32_t)arg;
  size_t bytes_read = i2s_bytes_to_read;

  while (1)
  {
    if (record_status)
    {
      /* 从I2S一次性读取数据 - 为XIAO ESP2S3 Sense和I2S.h库修改 */
      i2s_read(I2S_IN_PORT, (void *)sampleBuffer, i2s_bytes_to_read, &bytes_read, 100);
      // esp_i2s::i2s_read(esp_i2s::I2S_NUM_0, (void *)sampleBuffer, i2s_bytes_to_read, &bytes_read, 100);

      if (bytes_read <= 0)
      {
        ei_printf("Error in I2S read : %d", bytes_read);
      }
      else
      {
        if (bytes_read < i2s_bytes_to_read)
        {
          ei_printf("Partial I2S read");
        }

        // 放大数据(否则声音太小)
        for (int x = 0; x < i2s_bytes_to_read / 2; x++)
        {
          sampleBuffer[x] = (int16_t)(sampleBuffer[x]) * 8;
        }

        audio_inference_callback(i2s_bytes_to_read);
      }
    }
    delay(1);
  }
  vTaskDelete(NULL);
}

/**
 * @brief 初始化推理结构体并设置/启动PDM
 *
 * @param[in] n_samples 样本数
 *
 * @return 成功返回true,失败返回false
 */
static bool microphone_inference_start(uint32_t n_samples)
{
  inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));

  if (inference.buffer == NULL)
  {
    return false;
  }

  inference.buf_count = 0;
  inference.n_samples = n_samples;
  inference.buf_ready = 0;

  ei_sleep(100);

  record_status = true;

  xTaskCreate(capture_samples, "CaptureSamples", 1024 * 32, (void *)sample_buffer_size, 10, NULL);

  return true;
}

/**
 * @brief 等待新数据
 *
 * @return 完成返回true
 */
static bool microphone_inference_record(void)
{
  bool ret = true;

  while (inference.buf_ready == 0)
  {
    delay(10);
  }

  inference.buf_ready = 0;
  return ret;
}

/**
 * @brief 获取原始音频信号数据
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
  numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);

  return 0;
}

/**
 * @brief 停止PDM并释放缓冲区
 */
static void microphone_inference_end(void)
{
  free(sampleBuffer);
  ei_free(inference.buffer);
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif

void mainChat(void *arg)
{
  // 获取access token
  String baidu_access_token = "";
  String qianfan_access_token = "";

  baidu_access_token = getAccessToken(baidu_api_key, baidu_secret_key);
  qianfan_access_token = getAccessToken(qianfan_api_key, qianfan_secret_key);

  while (1)
  {
    if (!record_status)
    {
      // 从INMP441录制音频
      // 分配内存
      uint8_t *pcm_data = (uint8_t *)ps_malloc(BUFFER_SIZE);
      if (!pcm_data)
      {
        Serial.println("Failed to allocate memory for pcm_data");
        return;
      }

      Serial.println("i2s_read");
      // 开始循环录音,将录制结果保存在pcm_data中
      size_t bytes_read = 0, recordingSize = 0, ttsSize = 0;
      int16_t data[512];
      size_t noVoicePre = 0, noVoiceCur = 0, noVoiceTotal = 0, VoiceCnt = 0;
      bool recording = true;

      while (1)
      {
        // 记录开始时间
        noVoicePre = millis();

        // i2s录音
        esp_err_t result = i2s_read(I2S_IN_PORT, data, sizeof(data), &bytes_read, portMAX_DELAY);
        memcpy(pcm_data + recordingSize, data, bytes_read);
        recordingSize += bytes_read;
        Serial.printf("%x recordingSize: %d bytes_read :%d\n", pcm_data + recordingSize, recordingSize, bytes_read);

        // 计算平均值
        uint32_t sum_data = 0;
        for (int i = 0; i < bytes_read / 2; i++)
        {
          sum_data += abs(data[i]);
        }
        sum_data = sum_data / bytes_read;
        Serial.printf("sum_data :%d\n", sum_data);

        // 判断无声音时间超过阈值时退出录音
        noVoiceCur = millis();
        if (sum_data < 15)
        {
          noVoiceTotal += noVoiceCur - noVoicePre;
        }
        else
        {
          noVoiceTotal = 0;
          VoiceCnt += 1;
        }
        Serial.printf("noVoiceCur :%d noVoicePre :%d noVoiceTotal :%d\n", noVoiceCur, noVoicePre, noVoiceTotal);

        if (noVoiceTotal > 1000)
        {
          recording = false;
        }

        if (!recording || (recordingSize >= BUFFER_SIZE - bytes_read))
        {
          Serial.printf("record done: %d", recordingSize);

          break;
        }
      }

      // 设置唤醒录音状态为true,此后可以唤醒
      record_status = true;

      // 一直没有说话则退出被唤醒状态
      if (VoiceCnt == 0)
      {
        digitalWrite(LED_BUILTIN, LOW); // LED关闭

        recordingSize = 0;

        // 释放内存
        free(pcm_data);

        continue;
      }

      if (recordingSize > 0)
      {
        // 音频转文本(语音识别API访问)
        String recognizedText = baiduSTT_Send(baidu_access_token, pcm_data, recordingSize);
        Serial.println("Recognized text: " + recognizedText);

        // 访问千帆大模型(LLM大模型API访问)
        String ernieResponse = baiduErnieBot_Get(qianfan_access_token, recognizedText.c_str());
        Serial.println("Ernie Bot response: " + ernieResponse);

        // 文本转音频tts并通过MAX98357A输出(语音合成API访问)
        baiduTTS_Send(baidu_access_token, ernieResponse);
        Serial.println("ttsSize: ");
        Serial.println(ttsSize);
      }

      // 释放内存
      free(pcm_data);

      // 设置唤醒录音状态为false,此后继续录音对话
      record_status = false;
    }
    delay(10);
  }
}

// 获取百度API访问令牌
String getAccessToken(const char *api_key, const char *secret_key)
{
  String access_token = "";
  HTTPClient http;

  // 创建http请求
  http.begin("https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=" + String(api_key) + "&client_secret=" + String(secret_key));
  int httpCode = http.POST("");

  if (httpCode == HTTP_CODE_OK)
  {
    String response = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    access_token = doc["access_token"].as<String>();

    Serial.printf("[HTTP] GET access_token: %s\n", access_token);
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

  return access_token;
}

String baiduSTT_Send(String access_token, uint8_t *audioData, int audioDataSize)
{
  String recognizedText = "";

  if (access_token == "")
  {
    Serial.println("access_token is null");
    return recognizedText;
  }

  // audio数据包需要进行Base64编码,数据量会增大1/3
  int audio_data_len = audioDataSize * sizeof(char) * 1.4;
  unsigned char *audioDataBase64 = (unsigned char *)ps_malloc(audio_data_len);
  if (!audioDataBase64)
  {
    Serial.println("Failed to allocate memory for audioDataBase64");
    return recognizedText;
  }

  // json包大小,由于需要将audioData数据进行Base64的编码,数据量会增大1/3
  int data_json_len = audioDataSize * sizeof(char) * 1.4;
  char *data_json = (char *)ps_malloc(data_json_len);
  if (!data_json)
  {
    Serial.println("Failed to allocate memory for data_json");
    return recognizedText;
  }

  // Base64编码音频数据
  encode_base64(audioData, audioDataSize, audioDataBase64);

  memset(data_json, '\0', data_json_len);
  strcat(data_json, "{");
  strcat(data_json, "\"format\":\"pcm\",");
  strcat(data_json, "\"rate\":16000,");
  strcat(data_json, "\"dev_pid\":1537,");
  strcat(data_json, "\"channel\":1,");
  strcat(data_json, "\"cuid\":\"57722200\",");
  strcat(data_json, "\"token\":\"");
  strcat(data_json, access_token.c_str());
  strcat(data_json, "\",");
  sprintf(data_json + strlen(data_json), "\"len\":%d,", audioDataSize);
  strcat(data_json, "\"speech\":\"");
  strcat(data_json, (const char *)audioDataBase64);
  strcat(data_json, "\"");
  strcat(data_json, "}");

  // 创建http请求
  HTTPClient http_client;

  http_client.begin("http://vop.baidu.com/server_api");
  http_client.addHeader("Content-Type", "application/json");
  int httpCode = http_client.POST(data_json);

  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      // 获取返回结果
      String response = http_client.getString();
      Serial.println(response);

      // 从json中解析对应的result
      DynamicJsonDocument responseDoc(2048);
      deserializeJson(responseDoc, response);
      recognizedText = responseDoc["result"].as<String>();
    }
  }
  else
  {
    Serial.printf("[HTTP] POST failed, error: %s\n", http_client.errorToString(httpCode).c_str());
  }

  // 释放内存
  if (audioDataBase64)
  {
    free(audioDataBase64);
  }

  if (data_json)
  {
    free(data_json);
  }

  http_client.end();

  return recognizedText;
}

// 从百度文心一言获取响应
String baiduErnieBot_Get(String access_token, String prompt)
{
  String ernieResponse = "";

  if (access_token == "")
  {
    Serial.println("access_token is null");
    ernieResponse = "获取access token失败";
    return ernieResponse;
  }

  if (prompt.length() == 0)
  {
    ernieResponse = "识别出错了";
    return ernieResponse;
  }

  // 角色设定
  prompt += "你是一个语音助手，类似朋友的角色进行回答下面的问题，并且要求最多20个字简短的回答。";

  // 创建http请求,添加访问url和头信息
  HTTPClient http;

  // 千帆大模型API
  const char *ernie_api_url = "https://aip.baidubce.com/rpc/2.0/ai_custom/v1/wenxinworkshop/chat/ernie-lite-8k?access_token=";

  http.begin(ernie_api_url + String(access_token));
  http.addHeader("Content-Type", "application/json");

  // 创建JSON文档
  DynamicJsonDocument doc(2048);

  // 创建messages数组
  JsonArray messages = doc.createNestedArray("messages");

  // 创建message对象并添加到messages数组
  JsonObject message = messages.createNestedObject();
  message["role"] = "user";
  message["content"] = prompt;

  // 添加其他字段
  doc["disable_search"] = false;
  doc["enable_citation"] = false;

  // 将JSON数据序列化为字符串
  String requestBody;
  serializeJson(doc, requestBody);

  // 发送http请求
  int httpCode = http.POST(requestBody);

  // 处理访问结果
  if (httpCode == HTTP_CODE_OK)
  {
    // 获取返回结果并解析
    String response = http.getString();
    Serial.println(response);

    DynamicJsonDocument responseDoc(2048);
    deserializeJson(responseDoc, response);

    ernieResponse = responseDoc["result"].as<String>();
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  // 结束http请求
  http.end();

  // 返回响应数据
  return ernieResponse;
}

void baiduTTS_Send(String access_token, String text)
{
  if (access_token == "")
  {
    Serial.println("access_token is null");
    return;
  }

  if (text.length() == 0)
  {
    Serial.println("text is null");
    return;
  }

  const int per = 1;
  const int spd = 5;
  const int pit = 5;
  const int vol = 10;
  const int aue = 6;

  // URL编码
  String encodedText = urlEncode(urlEncode(text));

  // 构建URL和http请求数据
  String url = "https://tsn.baidu.com/text2audio";

  const char *header[] = {"Content-Type", "Content-Length"};

  url += "?tok=" + access_token;
  url += "&tex=" + encodedText;
  url += "&per=" + String(per);
  url += "&spd=" + String(spd);
  url += "&pit=" + String(pit);
  url += "&vol=" + String(vol);
  url += "&aue=" + String(aue);
  url += "&cuid=esp32s3";
  url += "&lan=zh";
  url += "&ctp=1";

  // 创建http请求
  HTTPClient http;

  http.begin(url);
  http.collectHeaders(header, 2);

  // 发送http请求
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0)
  {
    if (httpResponseCode == HTTP_CODE_OK)
    {
      String contentType = http.header("Content-Type");
      Serial.println(contentType);
      if (contentType.startsWith("audio"))
      {
        Serial.println("合成成功");

        // 获取返回的音频数据流
        Stream *stream = http.getStreamPtr();
        uint8_t buffer[512];
        size_t bytesRead = 0;

        // 设置timeout为200ms避免最后出现杂音
        stream->setTimeout(200);

        while (http.connected() && (bytesRead = stream->readBytes(buffer, sizeof(buffer))) > 0)
        {
          // 音频输出
          playAudio(buffer, bytesRead);
          delay(1);
        }

        // 清空I2S DMA缓冲区
        clearAudio();
      }
      else if (contentType.equals("application/json"))
      {
        Serial.println("合成出现错误");
      }
      else
      {
        Serial.println("未知的Content-Type");
      }
    }
    else
    {
      Serial.println("Failed to receive audio file");
    }
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
