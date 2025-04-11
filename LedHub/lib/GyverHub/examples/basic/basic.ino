// базовый пример работы с библиотекой, основные возможности. Загрузи и изучай код

// логин-пароль от роутера
#define AP_SSID "Alex"
#define AP_PASS "alexpass"

#include <Arduino.h>
#include <GyverHub.h>
GyverHub hub;
// GyverHub hub("MyDevices", "ESP", "");  // можно настроить тут, но без F-строк!

// обработчик кнопки (см. ниже)
void btn_cb() {
    Serial.println("click 4");
}

// обработчик кнопки с информацией о билде (см. ниже)
void btn_cb_b(gh::Build& b) {
    Serial.print("click 5 from client ID: ");
    Serial.println(b.client.id);
}

// билдер
void build(gh::Builder& b) {
    // =================== КОНТЕЙНЕРЫ ===================

    // основной контейнер страницы - столбец, т.е. виджеты будут располагаться на новых строках
    b.Button();
    b.Button();

    // СПОСОБ 1
    // вот так можно сделать горизонтальный контейнер
    b.beginRow();  // начать
    b.Button();
    b.Button();
    b.endRow();  // ВАЖНО НЕ ЗАБЫТЬ ЕГО ЗАВЕРШИТЬ

    // СПОСОБ 2
    // для красоты и удобства можно обернуть контейнер в блок {}
    // функции beginRow() и beginCol() всегда возвращают true
    if (b.beginRow()) {
        b.Button();
        b.Button();

        b.endRow();  // завершить
    }

    // СПОСОБ 3
    // есть ещё такой трюк - контейнер сам себя закроет в рамках блока {}
    // назвать его можно как угодно, передать во внутрь аргумент билдера
    {
        gh::Row r(b);  // контейнер сам создастся здесь
        b.Button();
        b.Button();
    }  // контейнер сам закроется здесь

    // СПОСОБ 4
    // и ещё вот такой макрос. Ширина контейнера 1 по умолчанию
    GH_ROW(b, 1,
           b.Button();
           b.Button(););

    // для вертикального контейнера справедлив такой же синтаксис:
    // beginCol(), endCol(), gh::Col(), GH_COL()

    // =================== ШИРИНА ===================

    // ширина виджетов задаётся в "долях" - отношении их ширины друг к другу
    // виджеты займут пропорциональное место во всю ширину контейнера
    // если ширина не указана - она принимается за 1
    {
        gh::Row r(b);
        b.Slider().size(3);  // слайдер шириной 3
        b.Button().size(1);  // кнопка шириной 1
        b.Button();          // тоже 1
        b.Button();
    }

    // посмотрим, как работает задание ширины контейнеру
    {
        gh::Row r(b);
        {
            // этот контейнер будет в 2 раза шире...
            gh::Row r(b, 2);
            b.Button();
            b.Button();
        }
        {
            // ...чем этот
            gh::Row r(b, 1);
            b.Button();
            b.Button();
        }
    }

    // =================== ПАРАМЕТРЫ ВИДЖЕТА ===================

    if (b.beginRow()) {
        // параметры виджета можно задавать цепочкой. Например:
        b.Button().label(F("my button 1")).color(gh::Colors::Red);

        // также можно продолжить настраивать ПРЕДЫДУЩИЙ виджет, обратившись к widget:
        b.Button();                        // кнопка без параметров
        b.widget.label(F("my button 2"));  // настраиваем кнопку выше
        b.widget.color(gh::Colors::Blue);  // её же

        b.endRow();
    }

    // =================== ДЕЙСТВИЯ ВИДЖЕТА ===================

    // с активных виджетов можно получить сигнал о действии - клик или изменение значения
    if (b.beginRow()) {
        // СПОСОБ 1
        // проверить условие click() - он вернёт true при действии
        // click() нужно вызывать ПОСЛЕДНИМ в цепочке!!!
        if (b.Button().click()) Serial.println("click 1");

        // СПОСОБ 2
        // подключить bool переменную - флаг
        bool flag = 0;
        b.Button().attach(&flag);
        if (flag) Serial.println("click 2");

        // СПОСОБ 3
        // подключить gh::Flag переменную - флаг
        // данный флаг сам сбросится в false при проверке!
        gh::Flag gflag;
        b.Button().attach(&gflag);
        if (gflag) Serial.println("click 3");
        // здесь gflag уже false

        // СПОСОБ 4
        // подключить функцию-обработчик (см. выше)
        b.Button().attach(btn_cb);

        // СПОСОБ 5
        // подключить функцию-обработчик с инфо о билде (см. выше)
        b.Button().attach(btn_cb_b);

        // attach() может быть НЕ ПОСЛЕДНИМ в цепочке, также их может быть несколько:
        // b.Button().attach(f1).attach(f2).label("kek");

        b.endRow();
    }

    // =============== ПОДКЛЮЧЕНИЕ ПЕРЕМЕННОЙ ===============

    if (b.beginRow()) {
        // библиотека позволяет подключить к активному виджету переменную для чтения и записи
        // я создам статические переменные для ясности. Они могут быть глобальными и так далее
        // таким образом изменения останутся при перезагрузке страницы
        static int sld;
        static String inp;
        static bool sw;

        // для подключения нужно передать переменную по адресу
        // библиотека сама определит тип переменной и будет брать из неё значение и записывать при действиях
        // библиотека поддерживает все стандартные типы данных, а также некоторые свои (Pairs, Pos, Button, Log...)
        b.Slider(&sld).size(2);
        b.Input(&inp).size(2);

        // внутри обработки действия переменная уже будет иметь новое значение:
        if (b.Switch(&sw).size(1).click()) {
            Serial.print("switch: ");
            Serial.println(sw);
        }

        b.endRow();
    }

    // ==================== ОБНОВЛЕНИЕ ====================

    // библиотека позволяет обновлять значения на виджетах. Это можно делать
    // - из основной программы (см. ниже)
    // - из билдера - но только при действиях по виджетам!

    // для отправки обновления нужно знать ИМЯ компонента. Его можно задать почти у всех виджетов
    // к функции добавляется подчёркивание, всё остальное - как раньше

    if (b.beginRow()) {
        b.Label_(F("label")).size(2).value("default");  // с указанием стандартного значения

        if (b.Button().size(1).click()) {
            hub.update(F("label")).value(random(100, 500));
        }

        b.endRow();
    }

    // в самом низу сделаем заголовок, текст будем обновлять из loop() (см. ниже)
    b.Title_(F("title"));

    // =================== ИНФО О БИЛДЕ ===================

    // можно получить информацию о билде и клиенте для своих целей
    // поставь тут 1, чтобы включить вывод =)
    if (0) {
        // запрос информации о виджетах
        if (b.build.isUI()) {
            Serial.println("=== UI BUILD ===");
        }

        // действие с виджетом
        if (b.build.isSet()) {
            Serial.println("=== SET ===");
            Serial.print("name: ");
            Serial.println(b.build.name);
            Serial.print("value: ");
            Serial.println(b.build.value);
        }

        Serial.print("client from: ");
        Serial.println(gh::readConnection(b.build.client.connection()));
        Serial.print("ID: ");
        Serial.println(b.build.client.id);
        Serial.println();
    }
}

void setup() {
    Serial.begin(115200);

#ifdef GH_ESP_BUILD
    // подключение к роутеру
    WiFi.mode(WIFI_STA);
    WiFi.begin(AP_SSID, AP_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println(WiFi.localIP());

    // если нужен MQTT - подключаемся
    hub.mqtt.config("test.mosquitto.org", 1883);
    // hub.mqtt.config("test.mosquitto.org", 1883, "login", "pass");

    // ИЛИ

    // режим точки доступа
    // WiFi.mode(WIFI_AP);
    // WiFi.softAP("My Hub");
    // Serial.println(WiFi.softAPIP());    // по умолч. 192.168.4.1
#endif

    // указать префикс сети, имя устройства и иконку
    hub.config(F("MyDevices"), F("ESP"), F(""));

    // подключить билдер
    hub.onBuild(build);

    // запуск!
    hub.begin();
}

void loop() {
    // =================== ТИКЕР ===================
    // вызываем тикер в главном цикле программы
    // он обеспечивает работу связи, таймаутов и прочего
    hub.tick();

    // =========== ОБНОВЛЕНИЯ ПО ТАЙМЕРУ ===========
    // в библиотеке предусмотрен удобный класс асинхронного таймера
    static gh::Timer tmr(1000);  // период 1 секунда

    // каждую секунду будем обновлять заголовок
    if (tmr) {
        hub.update(F("title")).value(millis());
    }
}