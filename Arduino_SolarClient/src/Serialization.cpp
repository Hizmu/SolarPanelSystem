// Serialization.cpp
#include "Serialization.h"

void serializeSensorData(uint8_t* buffer, const SensorData& data)
{
    int index = 3;
    buffer[index++] = (data.temperature >> 8) & 0xFF;
    buffer[index++] = data.temperature & 0xFF;

    buffer[index++] = (data.humidity >> 8) & 0xFF;
    buffer[index++] = data.humidity & 0xFF;

    buffer[index++] = (data.current >> 8) & 0xFF;
    buffer[index++] = data.current & 0xFF;

    buffer[index++] = (data.voltage >> 8) & 0xFF;
    buffer[index++] = data.voltage & 0xFF;
}

void deserializeSensorData(const uint8_t* buffer, SensorData& data)
{
    int index = 3;
    data.temperature = (buffer[index++] << 8) | buffer[index++];
    data.humidity = (buffer[index++] << 8) | buffer[index++];
    data.current = (buffer[index++] << 8) | buffer[index++];
    data.voltage = (buffer[index++] << 8) | buffer[index++];
}

