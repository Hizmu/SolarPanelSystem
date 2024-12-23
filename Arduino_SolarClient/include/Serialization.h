// Serialization.h
#pragma once
#include "common.h"

void serializeSensorData(uint8_t* buffer, const SensorData& data);
void deserializeSensorData(const uint8_t* buffer, SensorData& data);