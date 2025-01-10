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
#include "Config.h"
#include "Storage.h"
#include "AudioManager.h" 
#include "NetworkManager.h"

// 声明 u8g2 对象
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// 使用类来管理UI状态
class UIManager {
private:
    RBD::Button upButton;
    RBD::Button downButton;
    RBD::Button enterButton;
    
    int menuLevel = 0;
    int position = 1;
    int L0_position = 1;
    int L1_position = 1;
    
public:
    UIManager() : 
        upButton(Config::UP_PIN, INPUT_PULLUP),
        downButton(Config::DOWN_PIN, INPUT_PULLUP),
        enterButton(Config::ENTER_PIN, INPUT_PULLUP) {
        
        upButton.setDebounceTimeout(50);
        downButton.setDebounceTimeout(50);
        enterButton.setDebounceTimeout(50);
    }
    
    void handleInput() {
        if(downButton.onPressed()) {
            position++;
            if(position > 4) position = 1;
        }
        
        if(upButton.onPressed()) {
            position--;
            if(position < 1) position = 4;
        }
        
        if(enterButton.onPressed()) {
            if(menuLevel != 0 && position == 4) {
                menuLevel--;
                position = 1;
            } else {
                L0_position = position;
                menuLevel++;
                position = 1;
            }
        }
    }
    
    int getMenuLevel() const { return menuLevel; }
    int getPosition() const { return position; }
    int getL0Position() const { return L0_position; }
    int getL1Position() const { return L1_position; }
    bool isEnterPressed() { return enterButton.onPressed(); }
};

// 主要业务逻辑类
class VoiceNote {
private:
    UIManager ui;
    WebServer server;
    bool STT_state = false;
    bool GPT_state = false;
    String emailText;
    String answer;
    
    void setupI2S() {
        AudioManager::begin();
    }
    
    void setupStorage() {
        Storage::begin(Config::EEPROM_SIZE);
    }
    
    void setupNetwork() {
        NetworkManager::beginAP("GPT_set");
    }
    
public:
    void begin() {
        Serial.begin(115200);
        
        setupStorage();
        setupI2S();
        
        u8g2.begin();
        u8g2.enableUTF8Print();
        u8g2.setFont(u8g2_font_wqy14_t_gb2312b);
    }
    
    void loop() {
        ui.handleInput();
        handleMenu();
        
        if(NetworkManager::isConnected()) {
            server.handleClient();
        }
    }
    
private:
    void handleMenu() {
        switch(ui.getMenuLevel()) {
            case 0:
                handleMainMenu();
                break;
            case 1: 
                handleFirstLevelMenu();
                break;
            case 2:
                handleSecondLevelMenu();
                break;
        }
    }
    
    void handleMainMenu() {
        switch(ui.getPosition()) {
            case 1:
                L0P1();
                break;
            case 2:
                L0P2();
                break;
            case 3:
                L0P3();
                break;
            case 4:
                L0P4();
                break;
        }
    }

    void handleFirstLevelMenu() {
        if (ui.getL0Position() == 1) {  // 开始提问
            handleQuestionMenu();
        }
        else if (ui.getL0Position() == 2) {  // 设置
            handleSettingsMenu();
        }
        else if (ui.getL0Position() == 3) {  // 其他功能
            handleOtherFunctionsMenu();
        }
        else if (ui.getL0Position() == 4) {  // 使用教程
            handleTutorialMenu();
        }
    }

    void handleSecondLevelMenu() {
        if (ui.getL1Position() == 1) {  // 信息输入
            handleInfoInputMenu();
        }
        else if (ui.getL1Position() == 2) {  // 音量设置
            handleVolumeMenu();
        }
        else if (ui.getL1Position() == 3) {  // 语速设置
            handleSpeedMenu();
        }
    }

    void handleQuestionMenu() {
        question_page();
        if (ui.isEnterPressed()) {
            recording_page();
            startRecording();
            while (!isRecordingComplete()) {
                if (ui.isEnterPressed()) {
                    stopRecording();
                    break;
                }
            }
            
            sendquestion_page();
            String question = performSTT();
            if (!question.isEmpty()) {
                answer = getGPTAnswer(question);
                emailText = answer;
                
                speech_page();
                playAnswer(answer);
            }
        }
    }

    void handleSettingsMenu() {
        switch(ui.getPosition()) {
            case 1:
                L1N1P1();
                break;
            case 2:
                L1N1P2();
                break;
            case 3:
                L1N1P3();
                break;
            case 4:
                L1N1P4();
                break;
        }
    }

    void handleOtherFunctionsMenu() {
        switch(ui.getPosition()) {
            case 1:
                L1N2P1();
                break;
            case 2:
                L1N2P2();
                break;
            case 3:
                L1N2P3();
                break;
            case 4:
                L1N2P4();
                break;
        }
    }

    void handleTutorialMenu() {
        videoQRcode();
        while (true) {
            if (ui.isEnterPressed()) {
                break;
            }
            delay(10);
        }
    }

    void handleInfoInputMenu() {
        u8g2.clearBuffer();
        u8g2.drawUTF8(0, 15, "在手机端连接WIFI名");
        u8g2.drawUTF8(0, 31, "GPT_set，在网页地");
        u8g2.drawUTF8(0, 47, "址栏输192.168.4.1");
        u8g2.drawUTF8(0, 62, "按键返回，详见教程");
        u8g2.sendBuffer();

        NetworkManager::disconnect();
        delay(200);
        setupWebServer();
        
        while (true) {
            server.handleClient();
            if (ui.isEnterPressed()) {
                break;
            }
        }
    }

    void handleVolumeMenu() {
        int volume = Storage::readByte(111);
        if (volume == Config::EMPTY_MARKER) {
            volume = 5;
        }

        while (true) {
            char buffer[16];
            sprintf(buffer, "%d", volume);
            
            u8g2.clearBuffer();
            u8g2.drawUTF8(0, 15, "播报音量设置为");
            u8g2.drawUTF8(0, 31, buffer);
            u8g2.drawUTF8(0, 47, "上下拨动调整音量");
            u8g2.drawUTF8(0, 62, "按下按键确认");
            u8g2.sendBuffer();

            if (ui.isEnterPressed()) {
                Storage::writeByte(volume, 111);
                break;
            }
            delay(10);
        }
    }

    void handleSpeedMenu() {
        int speed = Storage::readByte(112);
        if (speed == Config::EMPTY_MARKER) {
            speed = 5;
        }

        while (true) {
            char buffer[16];
            sprintf(buffer, "%d", speed);
            
            u8g2.clearBuffer();
            u8g2.drawUTF8(0, 15, "播报语速设置为");
            u8g2.drawUTF8(0, 31, buffer);
            u8g2.drawUTF8(0, 47, "上下拨动调整语速");
            u8g2.drawUTF8(0, 62, "按下按键确认");
            u8g2.sendBuffer();

            if (ui.isEnterPressed()) {
                Storage::writeByte(speed, 112);
                break;
            }
            delay(10);
        }
    }

private:
    void setupWebServer() {
        NetworkManager::beginAP("GPT_set");
        NetworkManager::setupWebServer(&server);
    }

    void startRecording() {
        // TODO: 实现录音开始逻辑
    }

    void stopRecording() {
        // TODO: 实现录音停止逻辑
    }

    bool isRecordingComplete() {
        // TODO: 实现录音完成检查逻辑
        return false;
    }

    String performSTT() {
        // TODO: 实现语音转文字逻辑
        return "";
    }

    String getGPTAnswer(const String& question) {
        // TODO: 实现获取GPT回答逻辑
        return "";
    }

    void playAnswer(const String& answer) {
        // TODO: 实现语音播放逻辑
    }
};

VoiceNote voiceNote;

void setup() {
    voiceNote.begin();
}

void loop() {
    voiceNote.loop();
}