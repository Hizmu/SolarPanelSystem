// main.cpp

#include <ETH.h>
#include <ESPAsyncWebServer.h>
#include "CommunicationManager.h"
#include "SolarWebServer.h"

#define RXD1 32
#define TXD1 33
const char* ssid = "SolarPanelLocal 192.168.1.5";

IPAddress local_IP(192, 168, 1, 2);    
IPAddress wifi_IP(192, 168, 1, 5);    
IPAddress gateway(192, 168, 1, 1);        
IPAddress subnet(255, 255, 255, 0);        
IPAddress dns(8, 8, 8, 8);                

const IPAddress plcIp(192, 168, 1, 3);        
const uint16_t plcPort = 2001;

AsyncWebServer webServer(80);

CommunicationManager commManager(plcIp, plcPort);

SolarWebServer server(webServer,commManager);

// Функція зворотного виклику для подій Ethernet
void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
        case SYSTEM_EVENT_ETH_START:
            SolarWebServer::addLog("ETH Started");
            ETH.setHostname("WT32-ETHE01");
            break;
        case SYSTEM_EVENT_ETH_CONNECTED:
            SolarWebServer::addLog("ETH Connected");
            break;
        case SYSTEM_EVENT_ETH_GOT_IP:
            SolarWebServer::addLog("ETH MAC: "); SolarWebServer::addLog(ETH.macAddress());
            SolarWebServer::addLog("ETH IP: "); SolarWebServer::addLog(ETH.localIP().toString());
            break;
        case SYSTEM_EVENT_ETH_DISCONNECTED:
            SolarWebServer::addLog("ETH Disconnected");
            break;
        case SYSTEM_EVENT_ETH_STOP:
            SolarWebServer::addLog("ETH Stopped");
            break;
        default:
            break;
    }
}


void setup() {
    Serial.begin(115200);
    Serial1.begin(115200,SERIAL_8N1, RXD1, TXD1);
    Serial2.begin(BAUD_RATE,SERIAL_8N1, RXD2, TXD2);

    delay(500); 
    
    if (!LittleFS.begin()) {
        SolarWebServer::addLog("An Error has occurred while mounting LittleFS");
        while (true) {
            delay(1000);
        }
    } else {
        SolarWebServer::addLog("LittleFS mounted successfully");
    }
    WiFi.softAPConfig(wifi_IP, wifi_IP, subnet);
    WiFi.softAP(ssid);
    
    WiFi.onEvent(WiFiEvent);
    if (!ETH.begin()) {
        SolarWebServer::addLog("ETH.begin() failed");
        while (true) {
            delay(1); 
        }
    } 
    if (!ETH.config(local_IP, gateway, subnet, dns)) {
        SolarWebServer::addLog("ETH Config Failed");
    }
    SolarWebServer::addLog(WiFi.softAPIP().toString());
    
    server.begin();
    SolarWebServer::addLog("Starting Ethernet ...");

}

void loop(){
    commManager.Update();

    delay(10); 
}
