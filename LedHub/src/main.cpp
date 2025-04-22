#include <Arduino.h>
#include <GyverHub.h>

/// @brief Объект для работы с GyverHub.
GyverHub Hub("MyDevices", "WiFiSetup", "f1eb");

const String NetworkStartMessage = "Обновите список...";

/// @brief Список найденных точек доступа.
String NetworkList = NetworkStartMessage;

/// @brief Индекс выбранной точки доступа.
uint8_t SelectedNetworkIndex;


String WiFiPassword;

String SelectedNetwork;

String ConnectionFailed = "e55d";
String ConnectionSucces = "e55c";
String ConnectionIcon = "f1e6";

bool WiFiConnected;

/// @brief Подключает к WiFi сети.
/// @param ssid Имя точки доступа.
/// @param password Пароль точки доступа.
void WiFiConnect(const String& ssid, const String& password);

/// @brief Раздает WiFi, ESP является точкой доступа. 192.168.4.1 - стандартный IP адрес ESP.
void WiFiDistribution();

/// @brief Ищет доступные точки доступа.
void ScanNetworks();

/// @brief Вызывается при подключении к WiFi сети.
void OnWiFiConnectionStarted()
{
  WiFiConnect(SelectedNetwork, WiFiPassword);
  Hub.update("ConnectionIcon").value(1);
}


/// @brief Вызывается при смене сети.
void OnNetworkChanged()
{
  uint8_t from = 0;
  uint8_t to = NetworkList.indexOf(';');
  for (int i = 0; i < SelectedNetworkIndex; i++)
  {
    from = to + 1;
    to = NetworkList.indexOf(';', from);
  }

  SelectedNetwork = NetworkList.substring(from, to);

  Serial.println(SelectedNetwork);
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
    builder.Select_("WiFiSelect", &SelectedNetworkIndex).size(3).noLabel(true).text(NetworkList).attach(OnNetworkChanged);
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
    builder.Title("Подключиться:").fontSize(20).align(gh::Align::Left).size(3);
    builder.Button().icon("f1e6").noLabel(true).attach(OnWiFiConnectionStarted).disabled(NetworkList == NetworkStartMessage).size(2).noTab(true);
    builder.Icon_("ConnectionIcon", &WiFiConnected).icon(ConnectionIcon).color(gh::Color(255)).disabled(NetworkList == NetworkStartMessage).noLabel(true).noTab(true).size(1);
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
  ScanNetworks();
}

void loop()
{
  Hub.tick();
}

void WiFiConnect(const String& ssid, const String& password)
{
  WiFi.disconnect(true);
  WiFi.begin(ssid, password);
  uint8_t timeout = 10;
  uint8_t observer = 0;
  WiFiConnected = false;
  while (observer < timeout) 
  {
    observer++;
    delay(500);
    Serial.print(".");

    if (WiFi.status() == WL_CONNECTED)
    {
      ConnectionIcon = ConnectionSucces;
      WiFiConnected = true;
      Serial.println();
      Serial.println(WiFi.localIP());
      // Hub.sendNotice("Успешное подключение");
      // Hub.sendRefresh();
      break;
    }
  }

  if (WiFiConnected)
  {
    Hub.sendNotice("Успешное подключение");
    Hub.sendRefresh();
    return;
  }
  // Serial.println();
  // Serial.println(WiFi.localIP());
  // String noticeMessage = "Успешное подключение к ";
  // noticeMessage += SelectedNetwork;
  // noticeMessage += '\n';
  // noticeMessage += WiFi.localIP();
  // Hub.sendNotice("Успешное подключение");

  String errorMessage = "Подключение не удалось: ";
  ConnectionIcon = ConnectionFailed;

  switch (WiFi.status())
  {
    case WL_NO_SSID_AVAIL:
    {
      errorMessage += "Имя сети неверное";
      break;
    }

    case WL_CONNECT_FAILED:
    {
      errorMessage += "Неуспешная попытка";
      break;
    }

    case WL_WRONG_PASSWORD:
    {
      errorMessage += "Пароль сети неверен";
      break;
    }

    case WL_DISCONNECTED:
    {
      errorMessage += "Отключение от точки доступа";
      break;
    }
  }

    Serial.println();
    Serial.println(errorMessage);
    Hub.sendAlert(errorMessage);
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
  OnNetworkChanged();
}