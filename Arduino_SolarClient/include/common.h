// common.h
#pragma once

#include <Arduino.h>
#include <ArduinoSTL.h>
#include <vector>
#include <cstring>

//#define _DEBUG
enum uart_ids {
    WT32_ETH01 = 10,
    ESP8266,
    Arduino
};

enum uart_data_types {
    solar_panel_feed_back_Type = 100,
    WIFI_Credentials_Type, //1
    ESP8266_Sensor_type, //2
    Arduino_Sensors_Type, //3 
    Simence_TCP_Status, //4
    ESP8266_STATUS_OK, //5
    ARDUINO_STATUS_OK, //6 
    ESP8266_Check_Connection, //7
    ARDUINO_Check_Connection, //8
    ARDUINO_ETH_Connection // 9
};

const uart_ids current_device = uart_ids::Arduino;

struct SensorData {
    uint16_t temperature;
    uint16_t humidity;
    uint16_t current;
    uint16_t voltage;
};

constexpr int PACKET_SIZE = 80;  
constexpr unsigned long BAUD_RATE = 9600;

#ifdef _DEBUG
    #define DPrint(data) Serial.println(data)
#else
    #define DPrint(data) 
#endif

struct Packet {
    uint8_t data[PACKET_SIZE];
    size_t length;
};
