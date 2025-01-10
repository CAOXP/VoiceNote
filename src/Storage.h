#pragma once

#include <EEPROM.h>
#include <Arduino.h>

class Storage {
public:
    static void begin(int size);
    static void writeString(const String& data, int position);
    static String readString(int position);
    static void writeByte(uint8_t data, int position);
    static uint8_t readByte(int position);

private:
    static bool initialized;
}; 