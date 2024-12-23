// PLCClient.cpp
#include "PLCClient.h"
#include <SolarWebServer.h>
PLCClient::PLCClient(const IPAddress plcIP,const uint16_t port):
    plc_ip(plcIP), plc_port(port)
{
}

void PLCClient::Update()
{
    if (!client.connected())
    {
        if (client.connect(plc_ip, plc_port, 1000))
        {
            SolarWebServer::addLog("Connected to PLC.");
            _isConnected = true;
        }
        else
        {
            SolarWebServer::addLog("Failed to connect to PLC.");
            _isConnected = false;
            return;
        }
    }

    if (_isConnected)
    {
        if (client.available() >= receive_packet_size)
        {
            uint8_t buffer[receive_packet_size];
            size_t bytesRead = client.read(buffer, receive_packet_size);
            if (bytesRead >= receive_packet_size)
            {
                parseReceivedData(buffer);
            }
        }

        if (sendDataStruct.Start || sendDataStruct.Stop|| sendDataStruct.AutoStartOff || 
            sendDataStruct.AutoStartOn || sendDataStruct.RemoteControlOn ||
            sendDataStruct.RemoteControlOff|| sendDataStruct.ResetPosition ||
            sendDataStruct.Left || sendDataStruct.Right || sendDataStruct.Up ||
            sendDataStruct.Down || sendDataStruct.SetSunsetTime || sendDataStruct.SetSunriseTime || 
            sendDataStruct.SetDateTime || sendDataStruct.SetVelocity)
        {
            send();
        }

    }
}

void PLCClient::sendData(const SolarPanelControlCommands& data)
{
    sendDataStruct = data;
}
void PLCClient::send()
{
    uint8_t serializedData[send_packet_size];
    memset(serializedData, 0, send_packet_size);
    
    serializeSolarPanelControl(serializedData, sendDataStruct);
    SolarWebServer::addLog("");
    sendPacket(serializedData, send_packet_size);
}
SolarPanelStatus PLCClient::getReceiveData() const
{
    return receiveDataStruct;
}

bool PLCClient::isConnected()const
{
    return _isConnected;
}

void PLCClient::sendPacket(const uint8_t* packet, size_t length)
{
    if (length > send_packet_size) {
        SolarWebServer::addLog("Packet length exceeds send_packet_size");
        return;
    }

    for(int i = 0; i < SolarPanelControlCommands_size; i++)
    {
        SolarWebServer::addLog(String(packet[i]));
    }

    SolarWebServer::addLog(""); 
    client.write(packet, length);
    clearSendData();
}

void PLCClient::parseReceivedData(const uint8_t* buffer)
{
    deserializeSolarPanelStatus(buffer, receiveDataStruct);
}

void PLCClient::clearSendData()
{
    memset(&sendDataStruct, 0, sizeof(SolarPanelControlCommands));
}
