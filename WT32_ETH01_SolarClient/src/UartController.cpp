// UartController.cpp
#include "UartController.h"
#include "CommunicationManager.h"
#include "SolarWebServer.h"
UartController::UartController(CommunicationManager *manager, HardwareSerial &serial, int id)
    : _manager(manager), _serial(serial)
{
    _id = id;
    onWaitResponse = false;
}

UartController::~UartController()
{

}

void UartController::Update()
{
    if (onWaitResponse)
    {
        unsigned long currentTime = millis();

        if (currentTime - lastRequestTime >= responseTimeout)
        {
            lastStatusConnected = false;
            SolarWebServer::addLog("Response timeout. Resending packet...");
            onWaitResponse = false;
        }
    }

    if (!onWaitResponse && _serial.availableForWrite() && !packetQueue.empty())
    {
        Packet packet = packetQueue.front();
        sendPacket(packet.data, packet.length);
        packetQueue.erase(packetQueue.begin());
        lastRequestTime = millis();
    }

    receivePacket();
}

void UartController::receivePacket()
{
    if (_serial.available() >= PACKET_SIZE)
    {
        uint8_t data[PACKET_SIZE];
        memset(data,0,sizeof(data));
        _serial.readBytes(data, PACKET_SIZE);
        printData(data, PACKET_SIZE);

        uint8_t receivedChecksum = data[PACKET_SIZE - 1];
        uint8_t calculatedChecksum = calculateChecksum(data, PACKET_SIZE);
        if (receivedChecksum != calculatedChecksum)
        {
            SolarWebServer::addLog("Checksum mismatch. Packet discarded.");
            while (_serial.available())
                _serial.read();
            return;
        }

        this->_manager->dataHandle(data);
    }
}
void UartController::sendCheckConnectionStatusESP8266()
{
    uint8_t serializedData[PACKET_SIZE];
    memset(serializedData, 0, PACKET_SIZE);
    serializedData[0] = uart_ids::ESP8266;
    serializedData[1] = uart_data_types::ESP8266_Check_Connection;
    enqueuePacket(serializedData, PACKET_SIZE);
}
void UartController::sendCheckConnectionStatusArduino()
{
    uint8_t serializedData[PACKET_SIZE];
    memset(serializedData, 0, PACKET_SIZE);
    serializedData[0] = uart_ids::Arduino;
    serializedData[1] = uart_data_types::ARDUINO_STATUS_OK;
    enqueuePacket(serializedData, PACKET_SIZE);
}

void UartController::sendPanelStatus(const SolarPanelStatus &data)
{
    uint8_t serializedData[PACKET_SIZE];
    memset(serializedData, 0, PACKET_SIZE);
    serializeSolarPanelStatus(uart_ids::ESP8266, serializedData, data);
    enqueuePacket(serializedData, PACKET_SIZE);
}

void UartController::sendWifiCredentials(const WiFiCredentials &credentials)
{
    uint8_t data[PACKET_SIZE];
    memset(data, 0, PACKET_SIZE);
    data[0] = uart_ids::ESP8266; 
    data[1] = uart_data_types::WIFI_Credentials_Type;
    SolarWebServer::addLog(credentials.ssid);
    SolarWebServer::addLog(credentials.password);
    serializeWifiCredentials(data, credentials);
    enqueuePacket(data, PACKET_SIZE);
}

void UartController::sendPLCConnected(bool isConnected)
{
    uint8_t serializedData[PACKET_SIZE];
    memset(serializedData, 0, PACKET_SIZE);
    serializedData[0] = uart_ids::ESP8266;
    serializedData[1] = uart_data_types::Simence_TCP_Status;
    serializedData[3] = isConnected;

    enqueuePacket(serializedData, PACKET_SIZE);
}
void UartController::sendSensorData(const SensorData &data)
{
    uint8_t serializedData[PACKET_SIZE];
    memset(serializedData, 0, PACKET_SIZE);
    serializedData[0] = uart_ids::ESP8266;
    serializedData[1] = uart_data_types::ESP8266_Sensor_type;

    serializeSensorData(serializedData, data);

    enqueuePacket(serializedData, PACKET_SIZE);
}

void UartController::sendPacket(uint8_t *packet, size_t length)
{

    if (length > PACKET_SIZE)
    {
        SolarWebServer::addLog("Packet length exceeds PACKET_SIZE");
        return;
    }
    packet[length - 1] = calculateChecksum(packet, length);
    SolarWebServer::addLog("sendPacket");
    SolarWebServer::addLog("id" + String(_id));

    _serial.write(packet, length);
    printData(packet,length);
    
}



void UartController::enqueuePacket(uint8_t *packetData, size_t length)
{

    if (length > PACKET_SIZE)
    {
        SolarWebServer::addLog("enqueuePacket: Packet length exceeds PACKET_SIZE");
        return;
    }
    Packet packet;
    memcpy(packet.data, packetData, length);
    packet.length = length;

    packetQueue.emplace_back(packet);
}

uint8_t UartController::calculateChecksum(const uint8_t *packet, size_t length)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < length - 1; i++)
    {
        sum += packet[i];
    }
    return static_cast<uint8_t>(sum & 0xFF);
}

void UartController::printData(const uint8_t *data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        SolarWebServer::addLog(String(data[i]));
    }
    SolarWebServer::addLog("");
}
