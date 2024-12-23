#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <LittleFS.h>
#include "SolarWebServer.h"

const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";

byte mac[] = {0xc4, 0xd8, 0xd5, 0x2d, 0x01, 0xe1};

IPAddress server(185, 237, 204, 60);
const uint16_t port = 5000;

AsyncWebServer webServer(80);

UartController uartController;
SolarWebServer solarServer(webServer, uartController);
WiFiCredentials currentCredentials = {"", ""};

constexpr unsigned long INTERVAL_WIFI_CREDENTIALS = 10000;
constexpr unsigned long INTERVAL_SENSOR_DATA = 5000;
constexpr unsigned long INTERVAL_UART_CHECK_CONNECTION = 5000;
constexpr unsigned long INTERVAL_PANEL_STATUS = 2000;
constexpr unsigned long INTERVAL_PANEL_DATA = 2000;
constexpr unsigned long INTERVAL_SERVER_CHECK_CONNECTION = 5000;
constexpr unsigned long INTERVAL_SERVER_SEND_DATA_CONNECTION = 15000;
constexpr unsigned long INTERVAL_SERVER_SEND_IP = 3000;
constexpr unsigned long INTERVAL_SEND_ETH_STATUS = 3000;

unsigned long previousUartCheckConnection = 0;
unsigned long previousPanelStatus = 0;
unsigned long previousWifiCredentials = 0;
unsigned long previousSensorData = 0;
unsigned long previousPanelData = 0;
unsigned long previousServerCheckConnection = 0;
unsigned long previousServerSendData = 0;
unsigned long previousSendIP = 0;
unsigned long previousSendETHStatus = 0;

bool serverConnected = false;
bool wifiConnected = false;
bool newSsid = false;
WiFiClient client;

void sendData()
{
    SensorData sensorData = uartController.sensorData();
    
    if (client.connect(server, port))
    {
        StaticJsonDocument<200> jsonDoc;
        jsonDoc["humidity"] = sensorData.humidity;
        jsonDoc["temperature"] = sensorData.temperature;
        jsonDoc["current"] = sensorData.current;
        jsonDoc["voltage"] = sensorData.voltage;
      
         jsonDoc["power"] = sensorData.voltage * sensorData.current;

        String postData;
        serializeJson(jsonDoc, postData);

        client.println("POST /api/sensor_data HTTP/1.1");
        client.println("Host: " + server.toString());
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(postData.length());
        client.println();
        client.println(postData);

        SolarWebServer::addLog("Дані успішно надіслані.");
    }
    else
    {
        SolarWebServer::addLog("Не вдалося підключитися до сервера.");
    }

    client.stop();
}

bool checkInternetConnection()
{
    SolarWebServer::addLog("Перевірка підключення до Інтернету...");
    bool connected = client.connect(server, port);
    if (connected)
    {
        SolarWebServer::addLog("Підключення до Інтернету успішне.");
        client.stop();
        return true;
    }
    else
    {
        SolarWebServer::addLog("Немає підключення до Інтернету.");
        SolarWebServer::addLog("Не вдалося підключитися до сервера.");
        return false;
    }
}
bool connectToWiFi(const String &ssid, const String &password, int &attempt)
{
    solarServer.end();
    WiFi.disconnect();
    SolarWebServer::addLog("Спроба підключення до WiFi ");
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startAttemptTime = millis();
    const unsigned long connectTimeout = 10000;

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < connectTimeout)
    {
        SolarWebServer::addLog(".");
        delay(500);
    }
    SolarWebServer::addLog("");

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected = true;
        SolarWebServer::addLog("Підключено до WiFi.");
        SolarWebServer::addLog("IP-адреса: ");
        SolarWebServer::addLog(WiFi.localIP().toString());
        solarServer.begin();
        return true;
    }
    else
    {
        wifiConnected = false;
        SolarWebServer::addLog("Не вдалося підключитися до WiFi.");
        attempt++;
        return false;
    }
}

bool scanAndConnect(const String &targetSSID, const String &targetPassword)
{
    int n = WiFi.scanNetworks();
    if (n == 0)
    {
        SolarWebServer::addLog("Не знайдено жодної мережі.");
        return false;
    }
    else
    {
        for (int i = 0; i < n; ++i)
        {
            if (WiFi.SSID(i) == targetSSID)
            {
                SolarWebServer::addLog("Знайдено цільову мережу. Спроба підключення...");
                int dummyAttempt = 0;
                return connectToWiFi(targetSSID, targetPassword, dummyAttempt);
            }
        }
        wifiConnected = false;
        SolarWebServer::addLog("Цільова мережа не знайдена.");
        return false;
    }
}

void setup()
{
    Serial.begin(BAUD_RATE, SERIAL_8N1);

    delay(500);
    LittleFS.begin();

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
}

void loop()
{
    station_status_t status = wifi_station_get_connect_status();
    unsigned long currentTime = millis();

    if (currentTime - previousUartCheckConnection >= INTERVAL_UART_CHECK_CONNECTION)
    {
        previousUartCheckConnection = currentTime;
        uartController.requestCheckConnection();
    }

    if (uartController.isUartConnected())
    {
        if (currentTime - previousWifiCredentials >= INTERVAL_WIFI_CREDENTIALS)
        {
            previousWifiCredentials = currentTime;
            uartController.requestWifiCredentials();
        }

        if (currentTime - previousPanelStatus >= INTERVAL_PANEL_STATUS)
        {
            previousPanelStatus = currentTime;
            uartController.requestPanelConnectionStatus();
        }
        if (currentTime - previousSensorData >= INTERVAL_SENSOR_DATA)
        {
            previousSensorData = currentTime;
            uartController.requestSensorData();
        }
        if (currentTime - previousSendIP >= INTERVAL_SERVER_SEND_IP && status == STATION_GOT_IP)
        {
            previousSendIP = currentTime;
            uartController.sendIPAddressStatus(WiFi.localIP());
        }
        if (currentTime - previousSendETHStatus >= INTERVAL_SEND_ETH_STATUS)
        {
            previousSendETHStatus = currentTime;

            uartController.sendETHStatus(status);
        }

        if (currentTime - previousPanelData >= INTERVAL_PANEL_DATA)
        {
            previousPanelData = currentTime;
            uartController.requestPanelData();
        }
    }

    WiFiCredentials receivedCredentials = uartController.getWifiCredentials();
    if (receivedCredentials.ssid != "" &&
        (receivedCredentials.ssid != currentCredentials.ssid ||
         receivedCredentials.password != currentCredentials.password))
    {
        currentCredentials = receivedCredentials;
        newSsid = true;
    }

    if (uartController.sensorDataReceived() && currentTime - previousServerCheckConnection >= INTERVAL_SERVER_CHECK_CONNECTION)
    {
        bool connected = checkInternetConnection();
        previousServerCheckConnection = currentTime;
        if (connected && currentTime - previousServerSendData >= INTERVAL_SERVER_SEND_DATA_CONNECTION)
        {
            sendData();
            previousServerSendData = currentTime;
        }
    }
    if ((!wifiConnected || newSsid) && currentCredentials.ssid != "")
    {
        scanAndConnect(currentCredentials.ssid, currentCredentials.password);
        newSsid = false;
    }

    uartController.update();
}
