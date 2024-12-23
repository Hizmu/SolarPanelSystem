// SolarWebServer.cpp
#include "SolarWebServer.h"

static String logs;

// Конструктор класу
SolarWebServer::SolarWebServer(AsyncWebServer &server, UartController &controller) : _server(server), _controller(controller) {}

// Ініціалізація маршрутів та обробників
void SolarWebServer::begin()
{
 
    _server.on(API_PANEL_STATUS, HTTP_GET, std::bind(&SolarWebServer::handleGetPanelData, this, std::placeholders::_1));
    _server.on(API_NETWORK, HTTP_GET, std::bind(&SolarWebServer::handleGetNetworkSettings, this, std::placeholders::_1));

    _server.on(API_PLC_CONNECTION_STATUS, HTTP_GET, std::bind(&SolarWebServer::handleCheckConnection, this, std::placeholders::_1));
    _server.on(API_SENSOR_DATA, HTTP_GET, std::bind(&SolarWebServer::handleSensorData, this, std::placeholders::_1));
    _server.on(API_DEBUG_LOGS, HTTP_GET, std::bind(&SolarWebServer::sendLog, this, std::placeholders::_1));
    // Обробка статичних файлів
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { 
                   request->send(LittleFS, INDEX_HTML, "text/html"); 
               });
    _server.on(STYLES_CSS, HTTP_GET, [](AsyncWebServerRequest *request)
               { 
                   request->send(LittleFS, STYLES_CSS, "text/css"); 
               });
    _server.on(SCRIPT_JS, HTTP_GET, [](AsyncWebServerRequest *request)
               { 
                   request->send(LittleFS, SCRIPT_JS, "application/javascript"); 
               });

    _server.onNotFound([](AsyncWebServerRequest *request)
                       {

                           JsonDocument doc;
                           doc["error"] = "Not Found";
                           String json;
                           serializeJson(doc, json);
                           request->send(404, "application/json", json); 
                       });

    _server.begin();
}
void SolarWebServer::end()
{
    _server.end();
}

void SolarWebServer::addLog(String str)
{
    DPrint(str);
    
    logs += "\n" + str;
}
void SolarWebServer::handleGetPanelData(AsyncWebServerRequest *request)
{
    JsonDocument responseDoc;

    SolarPanelStatus status = _controller.paneStatus();
    responseDoc["started"] = status.Started;
    responseDoc["autoStart"] = status.AutoStart;
    responseDoc["manualMode"] = status.ManualMode;
    responseDoc["hMoveDone"] = status.H_MoveDone;
    responseDoc["hMoving"] = status.H_Moving;
    responseDoc["vMovedone"] = status.V_MoveDone;
    responseDoc["vMovving"] = status.V_Moving;
    responseDoc["sunlightFounded"] = status.Sunlight_Founded;
    responseDoc["hPosition"] = status.H_Position;
    responseDoc["vPosition"] = status.V_Position;
    responseDoc["velocity"] = status.Velocity;
    responseDoc["sunSensor"] = status.SunSensor;
    
    sendJSONResponse(request, responseDoc);
}

// Обробник для отримання налаштувань мережі
void SolarWebServer::handleGetNetworkSettings(AsyncWebServerRequest *request)
{
    JsonDocument responseDoc;

    WiFiCredentials creds = _controller.getWifiCredentials();
    responseDoc["success"] = true;
    responseDoc["ssid"] = creds.ssid;
    responseDoc["password"] = creds.password;

    sendJSONResponse(request, responseDoc);
}


void SolarWebServer::sendLog(AsyncWebServerRequest *request)
{
    JsonDocument responseDoc;
    responseDoc["success"] = true;
    responseDoc["logs"] = logs;
    logs = "";
    sendJSONResponse(request, responseDoc);
}
void SolarWebServer::handleSensorData(AsyncWebServerRequest *request)
{
    JsonDocument responseDoc;

    SensorData data = _controller.sensorData();
    responseDoc["success"] = true;
    responseDoc["temperature"] = data.temperature;
    responseDoc["humidity"] = data.humidity;
    responseDoc["current"] = data.current;
    responseDoc["voltage"] = data.voltage;

    sendJSONResponse(request, responseDoc);
}
void SolarWebServer::handleCheckConnection(AsyncWebServerRequest *request)
{
    JsonDocument doc;
    doc["isPlcConnected"] = _controller.isConnectedToPlc();
    sendJSONResponse(request,doc);

}
void SolarWebServer::sendJSONResponse(AsyncWebServerRequest *request, JsonDocument &doc)
{
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
}

void SolarWebServer::sendErrorResponse(AsyncWebServerRequest *request, const char *message, int code)
{
    JsonDocument doc;
    doc["success"] = false;
    doc["message"] = message;
    String json;
    serializeJson(doc, json);
    request->send(code, "application/json", json);
}

String SolarWebServer::urlDecode(uint8_t* data, size_t len) {
    String input = "";
    for (size_t i = 0; i < len; i++)
    {
        input += (char)data[i];
    }
    
    String decoded = "";
    for (size_t i = 0; i < input.length(); i++) {

        if (input[i] == '%') {
            int value;
            sscanf(input.substring(i + 1, i + 3).c_str(), "%x", &value);
            decoded += (char)value;
            i += 2;
        } else if (input[i] == '+') {
            decoded += ' ';
        } else{
            decoded += input[i];
        }
    }
    return decoded;
}