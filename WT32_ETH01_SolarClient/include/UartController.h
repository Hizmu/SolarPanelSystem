// UartController.h
#pragma once

#include "Serialization.h"

class CommunicationManager;

class UartController
{
private:
    bool onWaitResponse = false;
    bool initialized = false;
    const uint8_t maxPointCount = 24;
    std::vector<Packet> packetQueue;
    unsigned long lastRequestTime = 0;
    const unsigned long responseTimeout = 2000;
    bool lastStatusConnected = false;
    HardwareSerial& _serial;
    CommunicationManager* _manager;
    int _id;
public:
    UartController(CommunicationManager* manager,HardwareSerial& serial, int id);
    ~UartController();

    void Update();
    void sendCheckConnectionStatusESP8266();
    void sendCheckConnectionStatusArduino();
    void sendPanelStatus(const SolarPanelStatus& data);
    void sendWifiCredentials(const WiFiCredentials& credentials);

    
    SolarPanelStatus getReceiveData() const;

    void sendPLCConnected(bool isConnected);
    void sendSensorData(const SensorData& data);

private:
    
    void sendOK();
    void sendPacket(uint8_t *packet, size_t length);
    void receivePacket();
    void dataHandle(uint8_t *data);
    void enqueuePacket(uint8_t *packetData, size_t length);
    uint8_t calculateChecksum(const uint8_t *packet, size_t length);
    void printData(const uint8_t *data, size_t length);
};
