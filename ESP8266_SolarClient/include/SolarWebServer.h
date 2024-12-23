// SolarWebServer.h
#pragma once

#include "UartController.h"

#define API_PANEL_STATUS "/api/panel/data"
#define API_NETWORK "/api/network/data"
#define API_PLC_CONNECTION_STATUS "/api/plc/connectionstatus"
#define API_SENSOR_DATA "/api/sensor_data"
#define API_DEBUG_LOGS "/logs"

#define INDEX_HTML "/index.html"
#define STYLES_CSS "/css/styles.css"
#define SCRIPT_JS "/js/scripts.js"


class SolarPanelControlCommands;

class SolarWebServer {
public:
    SolarWebServer(AsyncWebServer& server, UartController &controller);
    void begin();
    void end();
    static void addLog(String str);
private:
    UartController& _controller;
    AsyncWebServer& _server;
    SolarPanelControlCommands _last_commands;

    void handleGetPanelData(AsyncWebServerRequest *request);
    void handleGetNetworkSettings(AsyncWebServerRequest *request);
    void handleCheckConnection(AsyncWebServerRequest *request);
    void handleSensorData(AsyncWebServerRequest *request);
    void sendJSONResponse(AsyncWebServerRequest *request, JsonDocument& doc);
    void sendErrorResponse(AsyncWebServerRequest *request, const char* message, int code = 400);
    void sendLog(AsyncWebServerRequest *request);
    static String urlDecode(uint8_t* data, size_t len);
};
