#include "NetworkManager.h"

WebServer* NetworkManager::webServer = nullptr;

void NetworkManager::beginAP(const char* ssid) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid);
}

bool NetworkManager::connect(const String& ssid, const String& password) {
    WiFi.begin(ssid.c_str(), password.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }
    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::disconnect() {
    WiFi.disconnect();
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String NetworkManager::getLocalIP() {
    return WiFi.localIP().toString();
}

void NetworkManager::setupWebServer(WebServer* server) {
    webServer = server;
    if (webServer) {
        webServer->on("/", handleRoot);
        webServer->on("/connect", HTTP_POST, handleConnect);
        webServer->begin();
    }
}

void NetworkManager::handleClient() {
    if (webServer) {
        webServer->handleClient();
    }
}

void NetworkManager::handleRoot() {
    // Implementation
}

void NetworkManager::handleConnect() {
    // Implementation
} 