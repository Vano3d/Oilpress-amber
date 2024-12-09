// Программа для отжима масла
// v 3.1
#include <Arduino.h>
#include <GyverIO.h>
#include <Wire.h>
#include <LittleFS.h>
#include "ADS1X15.h"
#include <SPI.h>
#include <EncButton.h>
#include "GyverTimer.h" 
#include <Blinker.h>
#include <ArduinoJson.h>
// #include <GyverSegment.h>
#include "Update.h"
#include <ElegantOTA.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <TFT_eSPI.h>
#include <ESPmDNS.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include <string.h>

#include "PT-Sans24.h"
#include "PT-Sans28.h"

AsyncWebSocket ws("/ws");
DNSServer dnsServer;
AsyncWebServer server(80);

JsonDocument doc;
JsonDocument sok;
JsonDocument pork;
String jsonString;
String jsonParams;

#include "myseed.h"
#include "settings.h"
#include "tft.h"
#include "funcs.h"


#include "html.h"
#include "handlers.h"

unsigned long lastTime = 0;
unsigned long timerDelay = 500;

String param;


void SendHTMLStream(AsyncWebServerRequest *request) {
  // Создаем потоковый ответ с типом "text/html" и буфером размером 8192 байт
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print(htmlStart);
  response->print(CSS_CONTENT);
  response->print(htmlMiddle);
  response->print("const data =" + jsonString + ";const paramsData = " + jsonParams + ";");
  response->print(JS_CONTENT);
  response->print("</body></html>");
  request->send(response);
}

void setup() {
  Serial.begin(115200);
  if (!LittleFS.begin(FORMAT_IF_FAILED)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
  display.showNumber(8888);
  Wire.begin();
  Wire.setClock(100000);
  digitalWrite(BUZZER, HIGH);
  pinMode(PUMP, OUTPUT); // маслопресс
  pinMode(STOPLED, OUTPUT);
  pinMode(STARTBUTTON, INPUT_PULLUP);
  pinMode(STOPBUTTON, INPUT_PULLUP);

  stopBtn.setBtnLevel(LOW);
  startBtn.setBtnLevel(LOW);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  eb.setEncReverse(0);
  eb.counter = 0; // сбросить счётчик энкодера

  timerSerialDelay.setInterval(500);
  timerIndicatorDelay.setInterval(200);
  processUpdTmr.setInterval(1000);



  // Подключение внешнего АЦП
  checkADCConnection();

  if (isADCConnected) {
  Serial.println("ADS connected :-)");

 } else {
    Serial.println("Failed to initialize ADS.");
  }



  delay(100);

  // Файл с параметрами отжима
  if (!LittleFS.exists("/sets.txt")) {
    File file = LittleFS.open("/sets.txt", "w");
    file.print(factorySettings);
    file.close();
  }
  File file = LittleFS.open("/sets.txt", "r");
  deserializeJson(doc, file); //  Ошибка десериализации обрабатывается в другом месте
  file.close();

  Serial.println();
  delay(100);

  // // Файл с настройками
   if (!LittleFS.exists("/params.txt")) {
    File f = LittleFS.open("/params.txt", "w");
    f.print(facttoryParams);
    f.close();
  }
  File f = LittleFS.open("/params.txt", "r");
  deserializeJson(sok, f); //  Ошибка десериализации обрабатывается в другом месте
  f.close();
  

  Serial.println();

  // tftMainScreen();

  serializeJson(doc, jsonString);
  serializeJson(sok, jsonParams);
  serializeJson(sok, Serial);
  Serial.println("");


   if (sok[0]["beeper"].as <bool>()) {
    tone(BUZZER, 2000, 100);
  }

  updParams();

   wifiConnectScreen();

  display.setBrightness(BRIGHT_7);
  

  ElegantOTA.begin(&server); 

  String ssidWeb = sok[0]["ssid"].as<String>();
  String passWeb = sok[0]["password"].as<String>();

  WiFi.begin(ssidWeb, passWeb);
  Serial.print("Подключение к WiFi");

  // Ожидаем подключения
  for (int i = 0; i < 6; i++) { // Ожидание 5 секунд
    if (WiFi.status() == WL_CONNECTED) { 
      Serial.println("Подключено к роутеру WiFi");
      Serial.print("IP-адрес: ");
      Serial.println(WiFi.localIP());
      isConnectedToRouter = true;
      break;
    }
    
    delay(1000);
    Serial.print(".");
  }
      // Если подключение не удалось, создаем точку доступа
      while (WiFi.status() != WL_CONNECTED && millis()<5000) {
    delay(100);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Подключено к роутеру WiFi");
    Serial.print("IP-адрес: ");
    Serial.println(WiFi.localIP());
    isConnectedToRouter = true;
    wifiScreenRouter();
  } else {
    Serial.println("Не удалось подключиться к WiFi, создаем точку доступа...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid, password);
    Serial.print("Точка доступа создала IP-адрес: ");
    Serial.println(WiFi.softAPIP());
    isConnectedToRouter = false;
    wifiScreenAP();
  }
  
  server.begin();

  Serial.println("mDNS responder started");
  // Initialize mDNS

 if (!MDNS.begin(dnsName)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
  }

  initWebSocket();

  // Если просто зашли на адрес контроллера, пушится всё что в SendHTML
  server.on("/", HTTP_GET, SendHTMLStream);

  // Восстановление заводских настроек
  server.on("/factory-reset", HTTP_GET, [](AsyncWebServerRequest * request) {
    handleFactoryReset(request);
  });

  // Если кликнули на кнопку "Сохранить", отправляется полученная data с json-ом и записывается в файл
  server.on("/sendfile", HTTP_POST, [](AsyncWebServerRequest * request) {
    handleSendFile(request);
  });

  // Если кликнули на кнопку "Сохранить параметры", отправляется полученная data с json-ом и записывается в файл
  server.on("/sendsettings", HTTP_POST, [](AsyncWebServerRequest * request) {
    handleSendSettings(request);
  });

  server.on("/startform", HTTP_GET, [](AsyncWebServerRequest * request) {
    handleStartForm(request);
  });

  server.on("/web-stop", HTTP_GET, [](AsyncWebServerRequest * request) {
    handleWebStop(request);
  });

  // server.on("/web-start", HTTP_GET, [](AsyncWebServerRequest *request)
  //           { handleWebStart(request); });

  // safetyTime = sok[0]["protection"].as <int> ();
  // beeperFlag = sok[0]["beeper"].as <bool> ();
  // sensorPressure = sok[0]["sensor"].as <int> ();

    myTime.end = 0;
    myTime.totalHour = 0;
    myTime.leftMins = 0;
    myTime.past.hours = 0;
    myTime.past.mins = 0;
    myTime.past.sec = 0;
}

void loop() {

  mySeed.updateSeed(chozenSeed);// обновляем класс MySeed регулярно
  currentStage = mySeed.getCurrentStage(myTime.current);

  if (ADS.isBusy() == false)
  {
    ADS.requestADC(0);
    sensor.pressure = constrain(ADS.getValue()/pressureDivider,0,1000);
  }

  ElegantOTA.loop();

  // находим длину массива выбранной культуры
  arrayLen = mySeed.length();

// находим всё время процесса - сумма всех времён в массиве
 myTime.end = mySeed.calcEndTime();

  // находим оставшееся время процесса
  myTime.left = (myTime.end - myTime.current); 

  // переменные для подсчёта оставшегося времени
  myTime.leftHour  = myTime.left  / 3600;
  myTime.leftMins = (myTime.left - myTime.leftHour * 3600) / 60;
  myTime.leftSec = myTime.left % 60;

  // длительность программы в часах и минутах
  myTime.totalHour = myTime.end / 3600;
  myTime.totalMins = (myTime.end - myTime.totalHour * 3600) / 60;

  // переменные для подсчёта прошедшего времени
  myTime.past.hours = myTime.current / 3600;
  myTime.past.mins = myTime.current / 60 - myTime.current / 3600 * 60;
  myTime.past.sec = myTime.current % 60;

  // server.handleClient();



  if (timerIndicatorDelay.isReady()) display.showNumber(sensor.pressure);


  // Коэффициены могут незначительно менять в зависимости от делителя
  switch (sensorPressure) {
  case 60:
    pressureDivider = PRESSURE60;
    break;

  case 70:
    pressureDivider = PRESSURE70;
    break;

  case 80:
    pressureDivider = PRESSURE80;
    break;

  default:
  pressureDivider = 1.0; // Значение по умолчанию
    break;
  }

  eb.tick();
  startBtn.tick();
  stopBtn.tick();
  stopLed.tick();


  /*
Прокрутка культур или диапазонов
  */
  if (eb.left() && !wasStartedFlag) encRotate(true);

  if (eb.right()) encRotate(false);

  // Переходы с главного экрана к диапазонам по клику энкодера

  if (eb.click() && !wasStartedFlag) {
    encClick();
  }

  // кнопка старта процесса
  if (startBtn.click() && !wasStartedFlag) { // не сработает, если находимся на аварийном экране или экране окончания
    switch (currentScreen) {
    case WIFIINFO: // если экран с WiFi, переходим на главный
      currentScreen = MAIN;
      tftMainScreen();
      break; 
    case ALARM:
    case END:
      break;
      // запускает процесс на других экранах
    default:
      stopLed.stop();
      currentScreen = PROCESS;
      startProcess();
      break;
    }
  }

  // кнопка принудительного окончания процесса
  if (stopBtn.click() && wasStartedFlag) {
    stopProcess();
    endScreen();
  }

  // если находимся на аварийном экране, то кнопка "стоп" переводит на главный экран
  if (currentScreen == ALARM) {
    if (stopBtn.click()) {
      stopLed.stop();
      tftMainScreen();
      currentScreen = MAIN;
    }
  }

  // если после окончания процесса нажать кнопку "стоп", ТО прекатится мигание stopLed
  if (stopBtn.press() && endFlag) {
    stopLed.stop();
  }

  /* если нажать на кнопку "старт", то поднимается флаг wasStartedFlag, время начинает бежать
  с нуля, запускается цикл назначения диапазона давлений
  */
  if (wasStartedFlag) myTime.current = millis() / 1000ul - myTime.beforeStart + continueTime;

  // главный алгоритм, поддерживающий давление в диапазоне minPress-maxPress
  if (sensor.pressure >= sensor.maxPress && sensor.isFilled) {
    pump_off();
    sensor.isFilled = 0;
  } else if (sensor.pressure <= sensor.minPress && sensor.isFilled == 0 && wasStartedFlag) {
    if(sensor.minPress!=0) {
      pump_on();
      sensor.isFilled = 1;
    } 
  }

  if (temp >= sensor.maxTemp && sensor.isWarmed) {
    heat_off();
    sensor.isWarmed = 0;
  } else if (temp <= sensor.minTemp && sensor.isWarmed == 0 && wasStartedFlag) {
    heat_on();
    sensor.isWarmed = 1;
  }

  // проходимся по массиву и меняем диапазон давлений по времени
    if (currentStage >= 0 && currentStage < mySeed.length()) {
        sensor.maxPress = mySeed.maxPress(currentStage);
        sensor.minPress = mySeed.minPress(currentStage);
        sensor.maxTemp = mySeed.maxTemp(currentStage);
        sensor.minTemp = mySeed.minTemp(currentStage);

    } else {
        // Обработка окончания всех этапов
        endFlag = true;
        stopProcess();
        // Дополнительные действия при окончании процесса
    }
  

  // отрубаем пресс по времени окончания
  if (myTime.current >= myTime.end && wasStartedFlag) {
    stopProcess();
    stopLed.blink(100, 200, 600);
    endScreen();
  }

  // пищит три раза после окончания отжима
  if (beeperFlag) buzzer();

  /*
  защита гидравлики: если помпа работает не останавливаясь safetyTime секунд, 
  то процесс прекращается = Аварийный режим
  */
  if (pumpOnTmr.isReady() && wasStartedFlag && safetyTime != 0) {
      stopProcess();
      alarmScreen();
      stopLed.blink(100, 200, 600);
  }


  // как часто отдавать данные в WebSocket на вебморду
  if ((millis() - lastTime) > timerDelay) { // 2 раза в секунду вроде
    String sensorReadings = getSensorReadings();
    notifyClients(sensorReadings);
    lastTime = millis();
  }

  // поднятие штока -- маслостанция включается до достижения давления 100 бар
  // if (stopBtn.hold() && !wasStartedFlag) {
  //   pump_on();
  //   wasStartedStockUp = true;
  // }
  
  // if (wasStartedStockUp && pressure > 100) {
  //   pump_off();
  //   was
  //   StartedStockUp = false;
  // }

// Если нажать Стоп дважды, потом зажать на третий раз, сбросятся настройки WIFi по умолчанию 
  if (stopBtn.step(2)) {
    resetWiFi();
    ESP.restart();
  } 

// вывод информации на экране процесса
  if (currentScreen == 4) {
    if (processUpdTmr.isReady()) {
      screenBeginFlag = true;
      tft.fillRect(20,110,32,30,TFT_BLACK);
      tft.setCursor(myTime.leftHour > 9 ? 21 : 38, 113);
      tft.print(myTime.leftHour);

      tft.fillRect(84, 110, 35, 35, TFT_BLACK);
      tft.setCursor(myTime.leftMins > 9 ? 84 : 99, 113);
      tft.print(myTime.leftMins);

      tft.fillRect(150, 110, 35, 35, TFT_BLACK);
      tft.setCursor(myTime.leftSec > 9 ? 150 : 165, 113);
      tft.print(myTime.leftSec);

      // tft.fillRect(20,182,180,35,TFT_BLACK);
      tft.setTextColor(TFT_ORANGE, TFT_BLACK);
      tft.setCursor(40, 185);
      tft.print(sensor.maxPress);
      tft.print(" - ");
      tft.print(sensor.minPress);

      tft.setCursor(40, 265);
      tft.print(sensor.maxTemp);
      tft.print(" - ");
      tft.print(sensor.minTemp);

    }
  }

  // Вывод в Serial
  if (timerSerialDelay.isReady() && DEBUG == 1) {

    Serial.println("========");

    // Serial.println(chozenSeed);
    Serial.print("Программа: ");
    Serial.println(doc[chozenSeed]["name"].as<const char *>());

    // Serial.print("Max-min press: ");
    // Serial.print(sensor.maxPress);
    // Serial.print("-");
    // Serial.println(sensor.minPress);
    // Serial.print("Max-min temp: ");
    // Serial.print(maxTemp);
    // Serial.print("-");
    // Serial.println(minTemp);
    Serial.print("Array lenght");
    Serial.println(arrayLen);
    Serial.print("End time: ");
    Serial.println(myTime.end);
    Serial.print("Press diaps: ");
    Serial.print(sensor.maxPress);
    Serial.print("-");
    Serial.println(sensor.minPress);
    Serial.print("Is filled: ");
    Serial.println(sensor.isFilled);

  }
  ws.cleanupClients();

  if (webStartFlag) {
    startProcess();
    webStartFlag = false;
  }
}