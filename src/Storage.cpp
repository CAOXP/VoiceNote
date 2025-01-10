#include "Storage.h"

bool Storage::initialized = false;

void Storage::begin(int size) {
    if (!initialized) {
        EEPROM.begin(size);
        initialized = true;
    }
}

void Storage::writeString(const String& data, int position) {
    if (!initialized) return;
    
    EEPROM.write(position, data.length());
    for (int i = 0; i < data.length(); i++) {
        EEPROM.write(position + i + 1, data[i]);
    }
    EEPROM.commit();
}

String Storage::readString(int position) {
    if (!initialized) return "";
    
    int length = EEPROM.read(position);
    String result;
    for (int i = 0; i < length; i++) {
        result += (char)EEPROM.read(position + i + 1);
    }
    return result;
}

void Storage::writeByte(uint8_t data, int position) {
    if (!initialized) return;
    EEPROM.write(position, data);
    EEPROM.commit();
}

uint8_t Storage::readByte(int position) {
    if (!initialized) return 0;
    return EEPROM.read(position);
} 