#pragma once

#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

// 主菜单页面
void L0P1() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "开始提问");
    u8g2.sendBuffer();
}

void L0P2() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "设置");
    u8g2.sendBuffer();
}

void L0P3() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "其他功能");
    u8g2.sendBuffer();
}

void L0P4() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "使用教程");
    u8g2.sendBuffer();
}

// 设置菜单页面
void L1N1P1() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "信息输入");
    u8g2.sendBuffer();
}

void L1N1P2() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "音量设置");
    u8g2.sendBuffer();
}

void L1N1P3() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "语速设置");
    u8g2.sendBuffer();
}

void L1N1P4() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "返回");
    u8g2.sendBuffer();
}

// 其他功能菜单页面
void L1N2P1() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "功能1");
    u8g2.sendBuffer();
}

void L1N2P2() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "功能2");
    u8g2.sendBuffer();
}

void L1N2P3() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "功能3");
    u8g2.sendBuffer();
}

void L1N2P4() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "返回");
    u8g2.sendBuffer();
}

// 其他页面
void videoQRcode() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "扫描二维码");
    u8g2.drawUTF8(0, 31, "观看使用教程");
    u8g2.drawUTF8(0, 47, "按下按键返回");
    u8g2.sendBuffer();
}

// 添加新的页面显示函数
void question_page() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "准备提问");
    u8g2.drawUTF8(0, 31, "按下按键开始录音");
    u8g2.sendBuffer();
}

void recording_page() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "正在录音");
    u8g2.drawUTF8(0, 31, "按下按键结束录音");
    u8g2.sendBuffer();
}

void sendquestion_page() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "正在处理语音");
    u8g2.drawUTF8(0, 31, "请稍等...");
    u8g2.sendBuffer();
}

void speech_page() {
    u8g2.clearBuffer();
    u8g2.drawUTF8(0, 15, "正在播放回答");
    u8g2.drawUTF8(0, 31, "按下按键可以跳过");
    u8g2.sendBuffer();
}