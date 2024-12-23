// UartController.cpp
#include "UartController.h"
#include "SolarWebServer.h"

UartController::UartController()
{

}

UartController::~UartController()
{
    Serial.end();
}

void UartController::update()
{

    if (Serial.availableForWrite() && !packetQueue.empty())
    {
        Packet packet = packetQueue.front();
        sendPacket(packet.data, packet.length);
        lastRequestTime = millis();
        packetQueue.erase(packetQueue.begin());
    }

    receivePacket();

}

void UartController::sendPanelCommands(const SolarPanelControlCommands &data)
{
    uint8_t serializedData[PACKET_SIZE];
    serializeSolarPanelControl(serializedData, data);
    enqueuePacket(serializedData, PACKET_SIZE);
}

const bool UartController::panelConnectionStatusRequested() const
{
    return _panelConnectionStatusRequested;
}
const bool UartController::panelStatusRequested() const
{
    return _panelStatusRequested;
}
const bool UartController::wifiCredentialsRequested() const
{
    return _wifiCredentialsRequested;
}
const bool UartController::sensorDataRequested() const
{
    return _sensorDataRequested;
}

const bool UartController::isUartConnected()const {
    return _lastStatusConnected;
}
void UartController::sendCheckConnection()
{
    uint8_t packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    packet[0] = uart_ids::WT32_ETH01;
    packet[1] = uart_data_types::ESP8266_Check_Connection;
    enqueuePacket(packet, PACKET_SIZE);
}

void UartController::sendOK()
{
    uint8_t packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    packet[0] = uart_ids::WT32_ETH01;
    packet[1] = uart_data_types::ESP8266_STATUS_OK;
    enqueuePacket(packet, PACKET_SIZE);
}

void UartController::requestCheckConnection()
{
    uint8_t packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    packet[0] = uart_ids::WT32_ETH01;
    packet[1] = uart_data_types::ESP8266_Check_Connection;
    enqueuePacket(packet, PACKET_SIZE);
}
void UartController::requestPanelData()
{
    uint8_t packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    packet[0] = uart_ids::WT32_ETH01;
    packet[1] = uart_data_types::solar_panel_feed_back_Type;
    enqueuePacket(packet, PACKET_SIZE);
}

void UartController::requestPanelConnectionStatus()
{
    uint8_t packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    packet[0] = uart_ids::WT32_ETH01;
    packet[1] = uart_data_types::Simence_TCP_Status;
    enqueuePacket(packet, PACKET_SIZE);
}

void UartController::requestWifiCredentials()
{
    uint8_t packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    packet[0] = uart_ids::WT32_ETH01;
    packet[1] = uart_data_types::WIFI_Credentials_Type;
    enqueuePacket(packet, PACKET_SIZE);
}
void UartController::requestSensorData()
{
    uint8_t packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    packet[0] = uart_ids::WT32_ETH01;
    packet[1] = uart_data_types::ESP8266_Sensor_type;
    enqueuePacket(packet, PACKET_SIZE);
}
void UartController::sendIPAddressStatus(IPAddress ip)
{
    uint8_t packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    packet[0] = uart_ids::WT32_ETH01;
    packet[1] = uart_data_types::ESP8266_IPAddress;
    packet[3]  = ip[0];
    packet[4]  = ip[1];
    packet[5]  = ip[2];
    packet[6]  = ip[3];
    enqueuePacket(packet, PACKET_SIZE);
}
void UartController::sendETHStatus(station_status_t status)
{
    uint8_t packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    packet[0] = uart_ids::WT32_ETH01;
    packet[1] = uart_data_types::ESP8266_STATUS;
    packet[3]  = status;
    enqueuePacket(packet, PACKET_SIZE);
}
void UartController::sendPacket(uint8_t *packet, size_t length)
{
    if (length > PACKET_SIZE)
    {
        SolarWebServer::addLog("Packet length exceeds PACKET_SIZE");
        return;
    }

    packet[length - 1] = calculateChecksum(packet, length);
    
    Serial.write(packet, length);



    onWaitResponse = true;
}

const SolarPanelStatus UartController::paneStatus() const
{
    return _status;
}
const bool UartController::isConnectedToPlc() const
{
    return _plcConnected;
}
const SensorData UartController::sensorData() const
{
    return _sensorData;
}
const WiFiCredentials UartController::getWifiCredentials() const
{
    return _credentials;
}

const bool UartController::sensorDataReceived()const
{
    return _sensorDataReceived;
}
void UartController::receivePacket()
{

    if (Serial.available() >= PACKET_SIZE)
    {
        uint8_t data[PACKET_SIZE];
        memset(data,0,sizeof(data));
        Serial.readBytes(data, PACKET_SIZE);
        //printData(data,PACKET_SIZE);
        uint8_t receivedChecksum = data[PACKET_SIZE - 1];
        uint8_t calculatedChecksum = calculateChecksum(data, PACKET_SIZE);
        if (receivedChecksum != calculatedChecksum)
        {
            SolarWebServer::addLog("Checksum mismatch. Packet discarded.");
            while (Serial.available())
                Serial.read();
            return;
        }
        this->dataHandle(data);

    }
}

void UartController::dataHandle(uint8_t *data)
{
    if (data[0] == current_device)
    {
        switch (data[1])
        {
        case solar_panel_feed_back_Type:
            deserializeSolarPanelStatus(data, _status);
            //Serial.println("1");
            sendOK();
            break;
        case WIFI_Credentials_Type:
            deserializeWifiCredentials(data, _credentials);
            //Serial.println("2");
            sendOK();
            break;
        case Simence_TCP_Status:
            _plcConnected = data[3];
            //Serial.println("3");
            sendOK();
            break;
        case ESP8266_Sensor_type:
            deserializeSensorData(data, _sensorData);
            //Serial.println("4");
            _sensorDataReceived = true;
            sendOK();
            break;
        case ESP8266_Check_Connection:
            //Serial.println("5");
            _lastStatusConnected = true;
        }
    }
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