// Serialization.h
#pragma once
#include "common.h"


void serializeSolarPanelControl(uint8_t* buffer, const SolarPanelControlCommands& data, bool toWT32 = false);

void deserializeWifiCredentials(const uint8_t* buffer, WiFiCredentials& credentials);

void serializeSolarPanelStatus(uart_ids id, uint8_t* buffer, const SolarPanelStatus& data);
void deserializeSolarPanelStatus(const uint8_t* buffer, SolarPanelStatus& data);
void deserializeBufferToSolarPanelControlWorld(const uint8_t* buffer, SolarPanelControlCommands& data);

void serializeSensorData(uint8_t* buffer, const SensorData& data);
void deserializeSensorData(const uint8_t* buffer, SensorData& data);