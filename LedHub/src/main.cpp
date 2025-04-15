#include <Arduino.h>
#include <GyverHub.h>
#include "env.hpp"

/// @brief Объект для работы с GyverHub.
GyverHub Hub("MyDevices", "Setup", "f1eb");

const String NetworkStartMessage = "Обновите список...";

/// @brief Список найденных точек доступа.
String NetworkList = NetworkStartMessage;

/// @brief Индекс выбранной точки доступа.
int SelectedNetwork;

String WiFiPassword;

/// @brief Подключает к WiFi сети.
/// @param ssid Имя точки доступа.
/// @param password Пароль точки доступа.
void WiFiConnect(const String& ssid, const String& password);

/// @brief Раздает WiFi, ESP является точкой доступа. 192.168.4.1 - стандартный IP адрес ESP.
void WiFiDistribution();

/// @brief Ищет доступные точки доступа.
void ScanNetworks();

String GetSelectedNetwork();

/// @brief Вызывается при подключении к WiFi сети.
void OnWiFiConnectionStarted()
{
  WiFiConnect(GetSelectedNetwork(), WiFiPassword);
  Hub.update("ConnectionIcon").value(1);
}


/// @brief Вызывается при смене сети.
void OnNetworkChanged()
{
  Serial.println(GetSelectedNetwork());
}

/// @brief Вызывается при перерисовке UI.
/// @param builder Объект, отвечающий за отрисовку.
void OnBuild(gh::Builder& builder)
{
  builder.Title("Добро пожаловать").align(gh::Align::Center).fontSize(25);

  builder.Space();

  builder.Title("Выберите домашнюю сеть:").align(gh::Align::Center).fontSize(25);
  if (builder.beginRow())
  {
    builder.Select_("WiFiSelect", &SelectedNetwork).size(3).noLabel(true).text(NetworkList).attach(OnNetworkChanged);
    builder.Button().icon("f2f1").size(1).attach(ScanNetworks).noLabel(true).noTab(true).color(gh::Color(255)).align(gh::Align::Right);
    builder.endRow();
  }

  if (builder.beginRow())
  {
    builder.Title("Пароль:").fontSize(20).align(gh::Align::Left).size(1);
    builder.Pass(&WiFiPassword).noLabel(true).size(2);
    builder.endRow();
  }

  if (builder.beginRow()) // && NetworkList != NetworkStartMessage
  {
    builder.Button().label("Подключится").attach(OnWiFiConnectionStarted).disabled(NetworkList == NetworkStartMessage);
    builder.Icon_("ConnectionIcon").icon("f1eb").color(gh::Color(255)).disabled(NetworkList == NetworkStartMessage);
    builder.endRow();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  WiFiDistribution();
  //WiFiConnect(WIFI_SSID, WIFI_PASS);

  
  Hub.onBuild(OnBuild);
  Hub.begin();
}

void loop()
{
  Hub.tick();
}

void WiFiConnect(const String& ssid, const String& password)
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      Serial.print(".");
  }

  Serial.println();
  Serial.println(WiFi.localIP());
}


void WiFiDistribution()
{
  String macAdress = WiFi.macAddress();
  macAdress.replace(":", "");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP_" + macAdress);
  Serial.println("192.168.4.1");
}

void ScanNetworks()
{
  Serial.println("Сканирую сеть");
  int networkCount = WiFi.scanNetworks(false, false);
  String result = "";
  for (int i = 0; i < networkCount; i++)
  {
    result += WiFi.SSID(i) + ";";
  }

  NetworkList = result;
  Hub.sendRefresh();
  Serial.println(result);
}

String GetSelectedNetwork()
{
  int counter = 0;
  int from = -1;
  int to = -1;
  for (int i = 0; i < NetworkList.length(); i++)
  {
    if (NetworkList[i] == ';')
    {
      counter++;
    }

    if (counter == SelectedNetwork && from == -1)
    {
      from = i + 1;
    }

    if (counter == SelectedNetwork + 1)
    {
      to = i;
      break;
    }
  }

  return NetworkList.substring(from, to);
}