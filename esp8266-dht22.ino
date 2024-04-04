#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#define SERVER_IP "192.168.10.199"
#define SERVER_PORT "1880"
#ifndef STASSID
#define STASSID "KKU-EE-IoT"
#define STAPSK "ee@kku1234"
#endif
#define DHTPIN D4
#define DHTTYPE DHT22

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WiFiMulti WiFiMulti;

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

void setup()
{
    Serial.begin(9600);
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(ssid, password);
    Serial.println();
    Serial.println();
    Serial.print("Wait for Connect to WiFi... ");
    while (WiFiMulti.run() != WL_CONNECTED)
    {
        Serial.print("#.");
        delay(1000);
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("SSID:");
    Serial.println(WiFi.SSID());
    Serial.println("Channel: ");
    Serial.println(WiFi.channel());
    Serial.println("MAC address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Subnet Masks: ");
    Serial.println(WiFi.subnetMask());
    Serial.println("Gateway IP: ");
    Serial.println(WiFi.gatewayIP());
    Serial.println("DNS address: ");
    Serial.println(WiFi.dnsIP());

    dht.begin();
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    dht.humidity().getSensor(&sensor);
    delayMS = sensor.min_delay / 1000;

    delay(1000);
}

void loop()
{
    delay(delayMS);
    sensor_t sensor;
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature))
    {
        Serial.println(F("Error reading temperature!"));
    }
    else
    {
        Serial.print(F("Temperature: "));
        Serial.print(event.temperature);
        Serial.println(F("°C"));
        
    }
    float temp = event.temperature;
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity))
    {
        Serial.println(F("Error reading humidity!"));
    }
    else
    {
        Serial.print(F("Humidity: "));
        Serial.print(event.relative_humidity);
        Serial.println(F("%"));
        
    }
    float humd = event.relative_humidity;
    String stemp = "";
    stemp += temp;
    String shumd = "";
    shumd += humd;
    if ((WiFi.status() == WL_CONNECTED))
    {
        WiFiClient client;
        HTTPClient http;
        Serial.print("[HTTP] begin...\n");
        http.begin(client, "http://" SERVER_IP ":" SERVER_PORT "/s1/");
        http.addHeader("Content-Type", "application/json");
        Serial.print("[HTTP] POST...\n");
        int httpCode = http.POST("{\"id\": \"10\",\"temp\": " + stemp + ",\"humd\": " + shumd + "}");
        if (httpCode > 0)
        {
            Serial.printf("[HTTP] POST... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK)
            {
                const String &payload = http.getString();
                Serial.println("received payload:\n<<");
                Serial.println(payload);
                Serial.println(">>");
            }
        }
        else
        {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
}