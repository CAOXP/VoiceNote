#pragma once

#include <driver/i2s.h>
#include "Config.h"

class AudioManager {
public:
    static void begin();
    static void record(uint16_t* buffer, size_t* length);
    static void play(const char* data, size_t length);
    static void stop();

private:
    static void i2sInit();
    static bool initialized;
}; 