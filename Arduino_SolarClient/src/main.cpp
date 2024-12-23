// main.cpp
#include <Ethernet.h>
#include "sensor.h"
#include <HardwareSerial.h>

byte mac[] = {0x34, 0x69, 0x50, 0xcf, 0x6c, 0x10};

IPAddress ip(192, 168, 0, 12);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress server(185, 237, 204, 60);
const uint16_t port = 5000;

UartController uart;

const unsigned long ethernetCheckInterval = 30000;
const unsigned long sensorInterval =  5000;
const unsigned long sendSensorData = 10000;
const unsigned long uartCheckConnection = 10000;

unsigned long previousMillisInternet = 0;
unsigned long previousMillisSensor = 0;
unsigned long previousMillisSendSensor = 0;
unsigned long previousUartCheckConnection = 0;

EthernetClient client;

bool isConnected = false;
bool ethSetuped = false;

void sendData(const SensorData& sensorData)
{
  if (client.connect(server, port))
  {
    String postData = "humidity=" + String(sensorData.humidity) +
                      "&temperature=" + String(sensorData.temperature) +
                      "&current=" + String(sensorData.current) +
                      "&voltage=" + String(sensorData.voltage) +
                       "&power=" + String(sensorData.voltage * sensorData.current);;

    client.println("POST /api/sensor_data HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);

    DPrint("Дані успішно надіслані.");
  }
  else
  {
    DPrint("Не вдалося підключитися до сервера.");
  }
  client.stop();
}

bool checkInternetConnection()
{
  DPrint("Перевірка підключення до Інтернету...");
  bool connected = client.connect(server, port);
  if (connected)
  {
    DPrint("Підключення до Інтернету успішне.");
    client.stop();
    return true;
  }
  else
  {
    DPrint("Немає підключення до Інтернету.");
    return false;
  }
}

void setup()
{
  dht.begin();


  Serial.begin(BAUD_RATE);
}
void setup_eth()
{

  if (Ethernet.begin(mac) == 0)
  {
    DPrint("Не вдалося налаштувати Ethernet за допомогою DHCP");

    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      DPrint("Ethernet shield не знайдено. Необхідно апаратне забезпечення.");
    }
    else if (Ethernet.linkStatus() == LinkOFF)
    {
      DPrint("Ethernet кабель не підключений.");
    }
    else
    {
      DPrint("Невідома помилка Ethernet.");
    }
  }

  if (checkInternetConnection())
  {
    ethSetuped = true;
  }

  DPrint("IP: ");
  DPrint(Ethernet.localIP());
}
void loop()
{
  SensorData sensorData = collectSensorData();
  unsigned long currentMillis = millis();
  if (currentMillis - previousUartCheckConnection >= uartCheckConnection)
  {
    previousUartCheckConnection = currentMillis;
    uart.uartCheckConnection();
  }
  
  if (currentMillis - previousMillisSensor >= sensorInterval && uart.isUartConnected())
  {
    previousMillisSensor = currentMillis;

    uart.sendSensorData(sensorData);
  }

  if (!ethSetuped)
  {
    setup_eth();
  }
  else if (currentMillis - previousMillisInternet >= ethernetCheckInterval)
  {
    previousMillisInternet = currentMillis;
    isConnected = checkInternetConnection();
  }


  if (currentMillis - previousMillisSendSensor >= previousMillisSendSensor && isConnected)
  {
    previousMillisSendSensor = currentMillis;

    sendData(sensorData);
  }

  uart.Update();
  
}
