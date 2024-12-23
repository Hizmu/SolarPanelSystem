// UartController.hpp
#pragma once

#include "Serialization.h"

class CommunicationManager;



class UartController
{
private:
    bool onWaitResponse = false;
    bool initialized = false;
    bool pointsReceived = false;
    bool sendTime = false;
    bool startedSendLiveBrightness = false;
    int pointCount = 0;
    const uint8_t maxPointCount = 24;
    std::vector<Packet> packetQueue;
    unsigned long lastRequestTime = 0;
    const unsigned long responseTimeout = 2000;
    bool _lastStatusConnected = false;

    SensorData _sensorData;
    SolarPanelStatus _status;
    WiFiCredentials _credentials;
    bool _plcConnected = false;
    bool _panelConnectionStatusRequested = false;
    bool _panelStatusRequested = false;
    bool _wifiCredentialsRequested = false;
    bool _sensorDataRequested = false;

    bool _sensorDataReceived = false;
public:
    UartController();
    ~UartController();

    void update();
    
    void sendPanelCommands(const SolarPanelControlCommands& data);
    void requestCheckConnection();
    void requestPanelConnectionStatus();
    void requestPanelData();
    void requestWifiCredentials();
    void requestSensorData();

    void sendIPAddressStatus(IPAddress ip);
    void sendETHStatus(station_status_t status);
    const SolarPanelStatus paneStatus() const;
    const bool isConnectedToPlc() const;
    const SensorData sensorData() const;
    const WiFiCredentials getWifiCredentials() const;

    const bool panelConnectionStatusRequested()const;
    const bool panelStatusRequested()const;
    const bool wifiCredentialsRequested()const;
    const bool sensorDataRequested()const;
    const bool isUartConnected()const;
    const bool sensorDataReceived()const; 
private:
    void sendCheckConnection();
    void sendOK();
    void sendPacket(uint8_t *packet, size_t length);
    void receivePacket();
    void dataHandle(uint8_t *data);
    void enqueuePacket(uint8_t *packetData, size_t length);
    void printData(const uint8_t *data, size_t length);
    uint8_t calculateChecksum(const uint8_t *packet, size_t length);
};
