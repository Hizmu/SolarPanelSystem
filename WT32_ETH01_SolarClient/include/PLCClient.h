// PLCClient.h
#pragma once
#include "UartController.h"


class PLCClient
{
public:
    PLCClient(const IPAddress plcIP,const uint16_t plcPort);

    void Update();

    void sendData(const SolarPanelControlCommands& data);
    SolarPanelStatus getReceiveData() const;

    bool isConnected()const;
private:
    void send();
    void sendPacket(const uint8_t* packet, size_t length);
    void receivePacket();
    void parseReceivedData(const uint8_t* buffer);
    void clearSendData();


    void serialize(uint8_t* buffer, const SolarPanelControlCommands& data);
    
    const size_t send_packet_size =  SolarPanelControlCommands_size;
    const size_t receive_packet_size = SolarPanelStatus_size;

    IPAddress plc_ip;
    uint16_t plc_port;
    SolarPanelControlCommands sendDataStruct;
    SolarPanelStatus receiveDataStruct;
    WiFiClient client;
    bool _isConnected = false;
};

