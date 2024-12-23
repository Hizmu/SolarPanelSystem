// UartController.h
#pragma once

#include "Serialization.h"

class UartController
{
private:
    bool onWaitResponse = false;
    bool initialized = false;
    int pointCount = 0;
    std::vector<Packet> packetQueue;
    unsigned long lastRequestTime = 0;
    const unsigned long responseTimeout = 5000;
    bool lastStatusConnected = false;

public:
    UartController();
    ~UartController();

    void Update();

    void sendETHConnected(bool isConnected);
    void sendSensorData(const SensorData& data);
    void SendETHConnected(bool isConnected);
    void uartCheckConnection();
    bool isUartConnected();
private:
    void RequestPoints();
    void sendPacket(uint8_t *packet, size_t length);
    void receivePacket();
    void dataHandle(uint8_t *data);
    void enqueuePacket(uint8_t *packetData, size_t length);
    uint8_t calculateChecksum(const uint8_t *packet, size_t length);
    void printData(const uint8_t *data, size_t length);
};
