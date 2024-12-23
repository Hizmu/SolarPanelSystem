// Serialization.cpp
#include "Serialization.h"
#include <SolarWebServer.h>

void _serializeSolarPanelControl(uint8_t* buffer, const SolarPanelControlCommands& data, size_t index ) {

    buffer[index++] = data.Start;
    buffer[index++] = data.Stop;
    buffer[index++] = data.AutoStartOn;
    buffer[index++] = data.AutoStartOff;
    buffer[index++] = data.RemoteControlOn;
    buffer[index++] = data.RemoteControlOff;
    buffer[index++] = data.Left;
    buffer[index++] = data.Right;
    buffer[index++] = data.Up;
    buffer[index++] = data.Down;
    buffer[index++] = data.ResetPosition;
    buffer[index++] = data.SetSunriseTime;
    buffer[index++] = data.SetSunsetTime;
    buffer[index++] = data.SetDateTime;
    buffer[index++] = data.SetVelocity;
    buffer[index++] = data.ToZeroPosition;
    // Serialize other fields
    buffer[index++] = data.sunrise_hours;
    buffer[index++] = data.sunrise_minutes;
    buffer[index++] = data.sunset_hours;
    buffer[index++] = data.sunset_minutes;
    buffer[index++] = data.weekday;
    buffer[index++] = data.day;
    buffer[index++] = data.month;

    // Serialize 16-bit integers
    buffer[index++] = data.year & 0xFF;
    buffer[index++] = (data.year >> 8) & 0xFF;

    buffer[index++] = data.hours;
    buffer[index++] = data.minutes;
    buffer[index++] = data.seconds;

    buffer[index++] = data.velocity & 0xFF;
    buffer[index++] = (data.velocity >> 8) & 0xFF;

}



void _deserializeSolarPanelControl(const uint8_t* buffer,SolarPanelControlCommands& data,  size_t index) {
    // Deserialize boolean values
    data.Start = buffer[index++];
    data.Stop = buffer[index++];
    data.AutoStartOn = buffer[index++];
    data.AutoStartOff = buffer[index++];
    data.RemoteControlOn = buffer[index++];
    data.RemoteControlOff = buffer[index++];
    data.Left = buffer[index++];
    data.Right = buffer[index++];
    data.Up = buffer[index++];
    data.Down = buffer[index++];
    data.ResetPosition = buffer[index++];
    data.SetSunriseTime = buffer[index++];
    data.SetSunsetTime = buffer[index++];
    data.SetDateTime = buffer[index++];
    data.SetVelocity = buffer[index++];
    data.ToZeroPosition = buffer[index++];
    // Deserialize other fields
    data.sunrise_hours = buffer[index++];
    data.sunrise_minutes = buffer[index++];
    data.sunset_hours = buffer[index++];
    data.sunset_minutes = buffer[index++];
    data.weekday = buffer[index++];
    data.day = buffer[index++];
    data.month = buffer[index++];

    // Deserialize 16-bit integers
    data.year = buffer[index++] | (buffer[index++] << 8);

    data.hours = buffer[index++];
    data.minutes = buffer[index++];
    data.seconds = buffer[index++];

    data.velocity = buffer[index++] | (buffer[index++] << 8);
}



void _deserializeSolarPanelStatus(const uint8_t* buffer, SolarPanelStatus& data, size_t index = 0) {

    data.Started = buffer[index++];
    data.AutoStart = buffer[index++];
    data.ManualMode = buffer[index++];
    data.H_MoveDone = buffer[index++];
    data.H_Moving = buffer[index++];
    data.V_MoveDone = buffer[index++];
    data.V_Moving = buffer[index++];
    data.Sunlight_Founded = buffer[index++];

    data.H_Position = buffer[index++] | (buffer[index++] << 8);
    data.V_Position = buffer[index++] | (buffer[index++] << 8);
    data.Velocity = buffer[index++] | (buffer[index++] << 8);
    data.SunSensor = buffer[index++] | (buffer[index++] << 8);
}

void _serializeSolarPanelStatus(uint8_t* buffer, const SolarPanelStatus& data, size_t index = 0) {

    buffer[index++] = data.Started;
    buffer[index++] = data.AutoStart;
    buffer[index++] = data.ManualMode;
    buffer[index++] = data.H_MoveDone;
    buffer[index++] = data.H_Moving;
    buffer[index++] = data.V_MoveDone;
    buffer[index++] = data.V_Moving;
    buffer[index++] = data.Sunlight_Founded;

    buffer[index++] = data.H_Position & 0xFF;
    buffer[index++] = (data.H_Position >> 8) & 0xFF;

    buffer[index++] = data.V_Position & 0xFF;
    buffer[index++] = (data.V_Position >> 8) & 0xFF;

    buffer[index++] = data.Velocity & 0xFF;
    buffer[index++] = (data.Velocity >> 8) & 0xFF;

    buffer[index++] = data.SunSensor & 0xFF;
    buffer[index++] = (data.SunSensor >> 8) & 0xFF;

}

void serializeSolarPanelControl(uint8_t* buffer, const SolarPanelControlCommands& data , bool toWT32) {

    size_t index = 0;
    if (toWT32)
    {   
        memset(buffer, 0, PACKET_SIZE);
        buffer[0] = uart_ids::WT32_ETH01; 
        buffer[1] = uart_data_types::solar_panel_feed_back_Type;
        buffer[2] = sizeof(SolarPanelControlCommands);
        index = 3;
  
    }else{
        memset(buffer, 0, SolarPanelControlCommands_size);
    }

    _serializeSolarPanelControl(buffer,data, index);

}

void serializeWifiCredentials(uint8_t* buffer, const WiFiCredentials& credentials)
{

    size_t index = 3;

    String serializedData = credentials.ssid + ";" + credentials.password;
    uint8_t dataLength = serializedData.length();

        if (dataLength > (PACKET_SIZE - 3)) {
        dataLength = PACKET_SIZE - 3; 
    }

    buffer[2] = dataLength; 

    serializedData.getBytes(&buffer[3], PACKET_SIZE - 3, 0);
}

void deserializeWifiCredentials(const uint8_t* buffer, WiFiCredentials& credentials)
{
    size_t index = 0;

    String ssid = String(reinterpret_cast<const char*>(&buffer[index]));
    credentials.ssid = ssid;

    index += ssid.length() + 1; 

    String password = String(reinterpret_cast<const char*>(&buffer[index]));
    credentials.password = password;

    SolarWebServer::addLog("WiFiCredentials deserialized:");
    SolarWebServer::addLog("SSID: " + credentials.ssid);
    SolarWebServer::addLog("Password: " + credentials.password);
}

void serializeSolarPanelStatus(uart_ids id, uint8_t* buffer, const SolarPanelStatus& data) {
    memset(buffer, 0, PACKET_SIZE);
    buffer[0] = id; 
    buffer[1] = uart_data_types::solar_panel_feed_back_Type;
    buffer[2] = sizeof(SolarPanelStatus);

    int16_t index = 3;
    _serializeSolarPanelStatus(buffer,data, index);
}


void deserializeSolarPanelStatus(const uint8_t* buffer, SolarPanelStatus& data) {
    size_t index = 3;

    _deserializeSolarPanelStatus(buffer,data,0);

}
void deserializeBufferToSolarPanelControlWord(const uint8_t* buffer, SolarPanelControlCommands& data) {
    int16_t index = 3;

    _deserializeSolarPanelControl(buffer,data,index);

}

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

