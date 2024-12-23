// SolarWebServer.cpp
#include "SolarWebServer.h"

static String logs;

// Конструктор класу
SolarWebServer::SolarWebServer(AsyncWebServer &server, CommunicationManager &manager) : _server(server), _manager(manager) {}

// Ініціалізація маршрутів та обробників
void SolarWebServer::begin()
{
    // Налаштування маршрутів
    _server.on(API_CONTROL_PANEL, HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               { this->handleControlPanel(request, data, len, index, total); });
    _server.on(API_PANEL_STATUS, HTTP_GET, std::bind(&SolarWebServer::handleGetPanelData, this, std::placeholders::_1));
    _server.on(API_NETWORK, HTTP_GET, std::bind(&SolarWebServer::handleGetNetworkSettings, this, std::placeholders::_1));
    _server.on(API_NETWORK_SAVE, HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               { this->handleSaveNetworkSettings(request, data, len, index, total); });
    _server.on(API_PLC_CONNECTION_STATUS, HTTP_GET, std::bind(&SolarWebServer::handleCheckConnection, this, std::placeholders::_1));
    _server.on(API_SENSOR_DATA, HTTP_GET, std::bind(&SolarWebServer::handleSensorData, this, std::placeholders::_1));
    _server.on(API_DEBUG_LOGS, HTTP_GET, std::bind(&SolarWebServer::handleLog, this, std::placeholders::_1));
    _server.on(API_ESP_STATUS, HTTP_GET, std::bind(&SolarWebServer::handleESPStatus, this, std::placeholders::_1));

    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(LittleFS, INDEX_HTML, "text/html"); });
    _server.on(STYLES_CSS, HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(LittleFS, STYLES_CSS, "text/css"); });
    _server.on(SCRIPT_JS, HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(LittleFS, SCRIPT_JS, "application/javascript"); });

    _server.onNotFound([](AsyncWebServerRequest *request)
                       {
                           JsonDocument doc;
                           doc["error"] = "Not Found";
                           String json;
                           serializeJson(doc, json);
                           request->send(404, "application/json", json); });

    _server.begin();
}

void SolarWebServer::addLog(String str)
{
    #ifdef _LOG
    logs += "\n" + str;
    #endif
    DPrint(str);
}

void SolarWebServer::handleControlPanel(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String body = urlDecode(data, len);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error)
    {
        SolarWebServer::addLog("JSON parsing failed");
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    SolarPanelControlCommands commands;
    commands.Start = doc["start"];
    commands.Stop = doc["stop"];
    commands.AutoStartOn = doc["autostartOn"];
    commands.AutoStartOff = doc["autostartOff"];

    commands.RemoteControlOn = doc["remoteOn"];
    commands.RemoteControlOff = doc["remoteOff"];

    commands.ResetPosition = doc["reset_position"];
    commands.ToZeroPosition = doc["zero_position"];
    SolarPanelStatus status = _manager.getPlcReceiveData();

    if (status.ManualMode)
    {
        const char *direction = doc["direction"];
        commands.Up = (strcmp(direction, "move_up") == 0);
        commands.Down = (strcmp(direction, "move_down") == 0);
        commands.Left = (strcmp(direction, "move_left") == 0);
        commands.Right = (strcmp(direction, "move_right") == 0);
    }
    else
    {
        commands.Up = false;
        commands.Down = false;
        commands.Left = false;
        commands.Right = false;
    }

    commands.SetDateTime = doc["set_datetime"];

    commands.day = doc["day"];
    commands.month = doc["month"];
    commands.year = doc["year"];
    commands.hours = doc["hours"];
    commands.minutes = doc["minutes"];
    commands.seconds = doc["seconds"];
    commands.weekday = doc["weekday"];
    commands.SetVelocity = doc["change_velocity"];
    commands.velocity = doc["velocity"];
    commands.SetSunriseTime = false;
    commands.SetSunsetTime = false;
    commands.sunrise_hours = 0;
    commands.sunrise_minutes = 0;
    commands.sunset_hours = 0;
    commands.sunset_minutes = 0;

    _manager.sendPlcData(commands);
    _last_commands = commands;

    JsonDocument responseDoc;
    responseDoc["success"] = true;
    responseDoc["message"] = "Commands processed successfully";
    sendJSONResponse(request, responseDoc);
}

void SolarWebServer::handleLog(AsyncWebServerRequest *request)
{
    JsonDocument responseDoc;
    responseDoc["success"] = true;
    responseDoc["logs"] = logs;
    logs = "";
    sendJSONResponse(request, responseDoc);
}

void SolarWebServer::handleGetPanelData(AsyncWebServerRequest *request)
{
    JsonDocument responseDoc;

    SolarPanelStatus status = _manager.getPlcReceiveData();
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
    WiFiCredentials creds;
    _manager.readCredentials(creds);
    responseDoc["ssid"] = creds.ssid;
    responseDoc["password"] = creds.password;

    sendJSONResponse(request, responseDoc);
}

void SolarWebServer::handleSaveNetworkSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String body = urlDecode(data, len);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error)
    {
        SolarWebServer::addLog("JSON parsing failed");
        sendErrorResponse(request, "Invalid JSON");
        return;
    }
    const char *ssid = doc["ssid"];
    const char *password = doc["password"];

    if (!ssid || !password)
    {
        sendErrorResponse(request, "Missing fields");
        return;
    }
    WiFiCredentials creds;
    creds.ssid = ssid;
    creds.password = password;

    _manager.saveCredentials(creds);

    bool success = true;
    const char *message = success ? "Network settings saved successfully" : "Failed to save network settings";

    JsonDocument responseDoc;
    responseDoc["success"] = success;
    responseDoc["message"] = message;

    sendJSONResponse(request, responseDoc);
}

void SolarWebServer::handleSensorData(AsyncWebServerRequest *request)
{
    JsonDocument responseDoc;

    SensorData data = _manager.sensorData();
    responseDoc["temperature"] = data.temperature;
    responseDoc["humidity"] = data.humidity;
    responseDoc["current"] = data.current;
    responseDoc["voltage"] = data.voltage;
    sendJSONResponse(request, responseDoc);
}
void SolarWebServer::handleCheckConnection(AsyncWebServerRequest *request)
{
    JsonDocument doc;
    doc["success"] = true;
    doc["isPlcConnected"] = _manager.getPLCClient().isConnected();
    sendJSONResponse(request, doc);
}
void SolarWebServer::handleESPStatus(AsyncWebServerRequest *request)
{
    JsonDocument doc;
    esp_status_t status = _manager.esp_status();
    String sStatus = "IDLE";
    switch (status)
    {
    case STATION_CONNECTING:
        sStatus = "Підключення ...";
        break;
    case STATION_WRONG_PASSWORD:
        sStatus = "Невірний пароль";
        break;
    case STATION_NO_AP_FOUND:
        sStatus = "Мережі не знайдено";
        break;
    case STATION_CONNECT_FAIL:
        sStatus = "Не вдалося з'єднатися";
        break;
    case STATION_GOT_IP:
        sStatus = "Підключено";
        break;
    default:
        sStatus = "IDLE";
        break;
    }
    doc["status"] = sStatus;
    doc["ip"] = _manager.espIPAddress().toString();
    sendJSONResponse(request, doc);
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

String SolarWebServer::urlDecode(uint8_t *data, size_t len)
{
    String input = "";
    for (size_t i = 0; i < len; i++)
    {
        input += (char)data[i];
    }

    String decoded = "";
    for (size_t i = 0; i < input.length(); i++)
    {

        if (input[i] == '%')
        {
            int value;
            sscanf(input.substring(i + 1, i + 3).c_str(), "%x", &value);
            decoded += (char)value;
            i += 2;
        }
        else if (input[i] == '+')
        {
            decoded += ' ';
        }
        else
        {
            decoded += input[i];
        }
    }
    return decoded;
}