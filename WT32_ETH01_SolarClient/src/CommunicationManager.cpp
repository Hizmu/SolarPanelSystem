#include "CommunicationManager.h"
#include "SolarWebServer.h"
CommunicationManager::CommunicationManager(const IPAddress plcIP, const uint16_t plcPort)
    : _plcClient(plcIP, plcPort), _esp8266Uart(this, Serial1,1), _arduinoUart(this, Serial2,2)
{
    readCredentials(_credentials);

}

void CommunicationManager::Update()
{
    _plcClient.Update();
    _esp8266Uart.Update();
    _arduinoUart.Update();
    

    
}

void CommunicationManager::dataHandle(uint8_t *buffer)
{
    if (buffer[0] == current_device)
    {
        switch (buffer[1])
        {
        case solar_panel_feed_back_Type:
            _esp8266Uart.sendPanelStatus(_plcClient.getReceiveData());
            break;
        case WIFI_Credentials_Type:
            readCredentials(_credentials);

            _esp8266Uart.sendWifiCredentials(_credentials);
            SolarWebServer::addLog("sendWifiCredentials");
            break;
        case ESP8266_Sensor_type:
            _esp8266Uart.sendSensorData(data);
            SolarWebServer::addLog("sendSensorData");
            break;
        case Arduino_Sensors_Type:
            deserializeSensorData(buffer, data);
            break;
        case Simence_TCP_Status:
            _esp8266Uart.sendPLCConnected(_plcClient.isConnected());
            SolarWebServer::addLog("sendPLCConnected");
            break;

        case ESP8266_Check_Connection:
            _esp8266Uart.sendCheckConnectionStatusESP8266();
            SolarWebServer::addLog("ESP8266_Connected");
            break;
        case ESP8266_IPAddress:
            _espip[0] = buffer[3];
            _espip[1] = buffer[4];
            _espip[2] = buffer[5];
            _espip[3] = buffer[6];
            SolarWebServer::addLog("ESP8266_IPAddress");
            break;
        case ESP8266_STATUS:
            _esp_status = (esp_status_t)buffer[3];
            SolarWebServer::addLog("ESP8266_STATUS");
            break;
        case ARDUINO_Check_Connection:
            _arduinoUart.sendCheckConnectionStatusArduino();
            SolarWebServer::addLog("Arduino_Connected");
            break;
        }
    }
}
esp_status_t CommunicationManager::esp_status() const
{
    return _esp_status;
}

IPAddress CommunicationManager::espIPAddress() const
{
    return _espip;
}
bool CommunicationManager::saveCredentials(const WiFiCredentials &creds)
{
    if (!LittleFS.begin())
        return false;

    File file = LittleFS.open("/wifi.dat", "w");
    if (!file)
        return false;

    file.println(creds.ssid);
    file.println(creds.password);

    file.close();

    this->_credentials = creds;
    return true;
}

bool CommunicationManager::readCredentials(WiFiCredentials &creds)
{
    if (!LittleFS.begin())
        return false;

    File file = LittleFS.open("/wifi.dat", "r");
    if (!file)
    {
        WiFiCredentials newCreads;
        newCreads.ssid = "MyNetwork";
        newCreads.password = "password123";

        saveCredentials(newCreads);
        creds = newCreads;
        return true;
    }
    if (file.available())
        creds.ssid = file.readStringUntil('\n');

    creds.ssid.trim();
    if (file.available())
    {
        creds.password = file.readStringUntil('\n');
        creds.password.trim();
    }
    file.close();
    return true;
}
const SensorData CommunicationManager::sensorData() const
{
    return data;
}
void CommunicationManager::sendPlcData(const SolarPanelControlCommands &data)
{
    _plcClient.sendData(data);
}

SolarPanelStatus CommunicationManager::getPlcReceiveData() const
{
    return _plcClient.getReceiveData();
}

const PLCClient &CommunicationManager::getPLCClient() const
{
    return _plcClient;
}

const UartController &CommunicationManager::esp8266Uart() const
{
    return _esp8266Uart;
}

const UartController &CommunicationManager::arduinoUart() const
{
    return _arduinoUart;
}
