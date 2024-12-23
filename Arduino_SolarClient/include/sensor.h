// sensor.h
#include <ACS712.h>
#include <DHT.h>
#include <UartController.h>

#define DHT_PIN 4
#define ACS_PIN A1
#define VOLTAGE_PIN A0

DHT dht(DHT_PIN, DHT22);
ACS712 acs(ACS_PIN,5,1023, 100);



float voltage = 0.0;
float current = 0.0;
float temperature = 0.0;
float humidity = 0.0;

SensorData collectSensorData()
{
    SensorData sensorData;
    humidity = dht.readHumidity();
    if (isnan(humidity))
    {
        humidity = 0.0;
    }
    sensorData.humidity = humidity;

    temperature = dht.readTemperature();
    if (isnan(temperature))
    {
        temperature = 0.0;
    }
    sensorData.temperature = temperature;

    current = acs.autoMidPoint();;
    sensorData.current = current;

    int analogValue = analogRead(VOLTAGE_PIN);
    voltage = analogValue * (5.0 / 1023.0);
    float inputVoltage = voltage * ((20 + 10) / 10);
    sensorData.voltage = inputVoltage;

    delay(1000);
    return sensorData;
}