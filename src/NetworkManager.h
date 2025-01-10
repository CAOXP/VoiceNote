#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include "Storage.h"

class NetworkManager {
public:
    static void beginAP(const char* ssid);
    static bool connect(const String& ssid, const String& password);
    static void disconnect();
    static bool isConnected();
    static String getLocalIP();
    
    static void setupWebServer(WebServer* server);
    static void handleClient();

private:
    static WebServer* webServer;
    static void handleRoot();
    static void handleConnect();
}; 