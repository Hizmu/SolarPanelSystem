// CommunicationManager.h
#pragma once
#include "PLCClient.h"

class AsyncWebServer;

class CommunicationManager
{
public:
    CommunicationManager(const IPAddress plcIP,const uint16_t plcPort);
    void Update();
    
    void dataHandle(uint8_t *data);
    void sendPlcData(const SolarPanelControlCommands& data);
    
    SolarPanelStatus getPlcReceiveData() const;

    
    const PLCClient& getPLCClient()const;
    const UartController& arduinoUart()const;
    const UartController& esp8266Uart()const;
    bool saveCredentials(const WiFiCredentials& credentials);
    bool readCredentials(WiFiCredentials &creds);
    esp_status_t esp_status()const;
    IPAddress espIPAddress()const;
    const SensorData sensorData()const;
private:
    bool ESP8266_ETH_Connected = false;
    bool Arduino_ETH_Connected = false;
    esp_status_t _esp_status = esp_status_t::STATION_IDLE;
    UartController _arduinoUart;
    UartController _esp8266Uart;
    PLCClient _plcClient;
    SensorData data;
    IPAddress _espip = IPAddress(0,0,0,0);
    WiFiCredentials _credentials;
};
