// SolarWebServer.h
#pragma once


#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "CommunicationManager.h"


#define API_CONTROL_PANEL "/api/panel/control"
#define API_PANEL_STATUS "/api/panel/data"
#define API_NETWORK "/api/network/data"
#define API_NETWORK_SAVE "/api/network/save"
#define API_SENSOR_DATA "/api/sensor/data"
#define API_PLC_CONNECTION_STATUS "/api/plc/connectionstatus"
#define API_DEBUG_LOGS "/logs"
#define API_ESP_STATUS "/esp/status"

#define INDEX_HTML "/index.html"
#define STYLES_CSS "/css/styles.css"
#define SCRIPT_JS "/js/scripts.js"


class SolarPanelControlCommands;

class SolarWebServer {
public:
    SolarWebServer(AsyncWebServer& server, CommunicationManager& manager);
    void begin();
    static void addLog(String str);
private:
    AsyncWebServer& _server;
    CommunicationManager& _manager;
    SolarPanelControlCommands _last_commands;

    void handleControlPanel(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleGetPanelData(AsyncWebServerRequest *request);
    void handleGetNetworkSettings(AsyncWebServerRequest *request);
    void handleSaveNetworkSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleCheckConnection(AsyncWebServerRequest *request);
    void handleSensorData(AsyncWebServerRequest *request);
    void handleESPStatus(AsyncWebServerRequest *request);
    void sendJSONResponse(AsyncWebServerRequest *request, JsonDocument& doc);
    void sendErrorResponse(AsyncWebServerRequest *request, const char* message, int code = 400);
    void handleLog(AsyncWebServerRequest *request);
    static String urlDecode(uint8_t* data, size_t len);
};
