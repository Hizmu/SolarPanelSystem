// common.h
#pragma once
#include <WiFiClient.h>
#include <Arduino.h>
#include <vector>
#include <cstring>
#include <LittleFS.h>

//#define _DEBUG
#define _LOG

enum uart_ids {
    WT32_ETH01 = 10,
    ESP8266,
    Arduino
};

typedef enum {
    STATION_IDLE = 0,
    STATION_CONNECTING,
    STATION_WRONG_PASSWORD,
    STATION_NO_AP_FOUND,
    STATION_CONNECT_FAIL,
    STATION_GOT_IP
} esp_status_t;

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
    ARDUINO_ETH_Connection, // 9
    ESP8266_IPAddress,
    ESP8266_STATUS
};

const uart_ids current_device = uart_ids::WT32_ETH01;

struct SolarPanelControlCommands {

    bool Start;
    bool Stop;
    bool AutoStartOn;
    bool AutoStartOff;
    bool RemoteControlOn;
    bool RemoteControlOff;
    bool Left;
    bool Right;

    bool Up;
    bool Down;
    bool ResetPosition;
    bool SetSunriseTime;
    bool SetSunsetTime;
    bool SetDateTime;
    bool SetVelocity;
    bool ToZeroPosition;
    
    uint8_t sunrise_hours;
    uint8_t sunrise_minutes;
    uint8_t sunset_hours;
    uint8_t sunset_minutes;
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint16_t velocity;
};
constexpr size_t SolarPanelControlCommands_size = 30;

struct SolarPanelStatus {
    bool Started;
    bool AutoStart;
    bool ManualMode;
    bool H_MoveDone;
    bool H_Moving;
    bool V_MoveDone;
    bool V_Moving;
    bool Sunlight_Founded;
    int16_t H_Position;
    int16_t V_Position;
    int16_t Velocity;
    int16_t SunSensor;
};

constexpr size_t SolarPanelStatus_size = 16;

struct WiFiCredentials {
    String ssid;      
    String password;  
};

struct SensorData {
    uint16_t temperature;
    uint16_t humidity;
    uint16_t current;
    uint16_t voltage;
};

struct ToServer{
    SensorData data;
};


constexpr int PACKET_SIZE = 80;  
constexpr int BAUD_RATE = 9600;
#define _DEBUG
#ifdef _DEBUG
    #define DPrint(data) Serial.println(data)
#else
    #define DPrint(data) 
#endif

struct Packet {
    uint8_t data[PACKET_SIZE];
    size_t length;
};
