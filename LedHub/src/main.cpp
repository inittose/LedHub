#include <Arduino.h>
#include <GyverHub.h>

/// @brief Объект для работы с GyverHub.
GyverHub Hub("MyDevices", "Setup", "f1eb");

/// @brief Список найденных точек доступа.
String NetworkList = "Обновите список...";

/// @brief Индекс выбранной точки доступа.
uint32_t SelectedNetwork = 0;

/// @brief Подключает к WiFi сети.
/// @param ssid Имя точки доступа.
/// @param password Пароль точки доступа.
void WiFiConnect(const String& ssid, const String& password);

/// @brief Подключает к WiFi сети. TODO: Назвать обработчиком.
void WiFiConnect();

/// @brief Раздает WiFi, ESP является точкой доступа. 192.168.4.1 - стандартный IP адрес ESP.
void WiFiDistribution();

/// @brief Ищет доступные точки доступа.
void ScanNetworks();

/// @brief Вызывается при смене сети.
void OnNetworkChanged()
{
  Serial.println(SelectedNetwork);
}

/// @brief Вызывается при перерисовке UI.
/// @param builder Объект, отвечающий за отрисовку.
void OnBuild(gh::Builder& builder)
{
  builder.Title("Выберите домашнюю сеть:").align(gh::Align::Center).fontSize(25);

  builder.Space();

  if (builder.beginRow())
  {
    builder.Select_("WiFiSelect", &SelectedNetwork).size(3).noLabel(true).text(NetworkList).attach(OnNetworkChanged);
    builder.Button().icon("f2f1").size(1).attach(ScanNetworks).noLabel(true).noTab(true).color(gh::Color(255)).align(gh::Align::Right);
    builder.endRow();
  }

  builder.Space();
  
  if (builder.beginRow())
  {
    builder.Title("Пароль:").fontSize(20).align(gh::Align::Left).size(1);
    builder.Pass().noLabel(true).size(2);
    builder.endRow();
  }

  builder.Space();

  if (builder.beginRow())
  {
    builder.Button().label("Подключится").attach(WiFiConnect);
    builder.Icon_("ConnectionIcon").icon("f1eb").color(gh::Color(255));
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

void WiFiConnect()
{
  WiFiConnect(WIFI_SSID, WIFI_PASS);
  Hub.update("ConnectionIcon").value(1);
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