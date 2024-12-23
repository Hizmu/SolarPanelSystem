// UartController.cpp
#include "UartController.h"
UartController::UartController()
{
}

UartController::~UartController()
{
    Serial.end();
}

void UartController::Update()
{
    if (onWaitResponse)
    {
        unsigned long currentTime = millis();

        if (currentTime - lastRequestTime >= responseTimeout)
        {
            lastStatusConnected = false;
            DPrint("Response timeout. Resending packet...");
            onWaitResponse = false;
        }
    }

    if (!onWaitResponse && Serial.availableForWrite() && !packetQueue.empty())
    {
        Packet packet = packetQueue.front();
        sendPacket(packet.data, packet.length);
        packetQueue.erase(packetQueue.begin()); 
        lastRequestTime = millis();
    }

    receivePacket();
}

void UartController::sendSensorData(const SensorData& data)
{
    uint8_t serializedData[PACKET_SIZE];
    memset(serializedData, 0, PACKET_SIZE);
    serializedData[0] = uart_ids::WT32_ETH01;
    serializedData[1] = uart_data_types::Arduino_Sensors_Type;
    serializedData[2] = sizeof(SensorData);
    serializeSensorData(serializedData, data);

    enqueuePacket(serializedData, PACKET_SIZE);
}
void UartController::SendETHConnected(bool isConnected)
{
    uint8_t serializedData[PACKET_SIZE];
    memset(serializedData, 0, PACKET_SIZE);
    serializedData[0] = uart_ids::WT32_ETH01;
    serializedData[1] = uart_data_types::ARDUINO_ETH_Connection;
    serializedData[3] = isConnected;

    enqueuePacket(serializedData, PACKET_SIZE);
}
void UartController::uartCheckConnection()
{
    uint8_t serializedData[PACKET_SIZE];
    memset(serializedData, 0, PACKET_SIZE);
    serializedData[0] = uart_ids::WT32_ETH01;
    serializedData[1] = uart_data_types::ARDUINO_Check_Connection;
    lastStatusConnected = false;
    onWaitResponse = false;
    packetQueue.clear();

    enqueuePacket(serializedData, PACKET_SIZE);
}
bool UartController::isUartConnected()
{
    return lastStatusConnected;
}

void UartController::sendPacket(uint8_t *packet, size_t length)
{
    if (length > PACKET_SIZE)
    {
        DPrint("Packet length exceeds PACKET_SIZE");
        return;
    }

    DPrint("sendPacket");
    
    packet[length - 1] = calculateChecksum(packet, length);
    printData(packet, length);
    Serial.write(packet, length);

    onWaitResponse = true;

}

void UartController::receivePacket()
{
    if (Serial.available())
    {
        uint8_t data[PACKET_SIZE];
        Serial.readBytes(data, PACKET_SIZE);
        DPrint("receivePacket");
        printData(data, PACKET_SIZE);

        uint8_t receivedChecksum = data[PACKET_SIZE - 1];
        uint8_t calculatedChecksum = calculateChecksum(data, PACKET_SIZE);
        if (receivedChecksum != calculatedChecksum)
        {
            DPrint("Checksum mismatch. Packet discarded.");
            return;
        }
        if(data[0] == current_device)
        {
            if(data[1] == uart_data_types::ARDUINO_STATUS_OK)
            {
                onWaitResponse = false;
                lastStatusConnected = true;     
            }
        }
    }
}


void UartController::enqueuePacket(uint8_t *packetData, size_t length)
{
    if (length > PACKET_SIZE)
    {
        DPrint("enqueuePacket: Packet length exceeds PACKET_SIZE");
        return;
    }
    Packet packet;
    memcpy(packet.data, packetData, length);
    packet.length = length;
    packetQueue.push_back(packet);
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
    DPrint("Length" + String(length));
    for (size_t i = 0; i < length; i++)
    {
        DPrint(data[i]);
    }
    
    DPrint("");
}
