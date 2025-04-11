#include <Arduino.h>
#include <GyverHub.h>

void WiFiConnect(const String& ssid, const String& password);
void WiFiDistribution(const String& netName);

void setup()
{
  Serial.begin(115200);
  WiFiDistribution("I'm_esp8266");
}

void loop()
{
  Serial.println("Hello!");
  delay(1000);
}

void WiFiConnect(const String& ssid, const String& password)
{
  WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println(WiFi.localIP());
}

void WiFiDistribution(const String& netName)
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(netName);
}