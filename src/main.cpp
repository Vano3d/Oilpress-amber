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

#include "settings.h"
#include "tft.h"
#include "funcs.h"

#include "html.h"
#include "handlers.h"

unsigned long lastTime = 0;
unsigned long timerDelay = 500;

String param;
uint32_t start, stop;
int16_t val_0;

void setup() {
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

  Serial.begin(115200);

  // Подключение внешнего АЦП
  checkADCConnection();

  if (isADCConnected) {
   ADS.begin();
   ADS.setGain(1);
      //  voltage factor

  start = micros();
  ADS.requestADC(0);
  stop = micros();
  // Serial.println(stop - start);
  delay(100);
  while (ADS.isBusy());
  start = micros();
  val_0 = ADS.getValue();
  stop = micros();
  // Serial.println(stop - start);
  delay(100);

  ADS.requestADC(0);

 } else {
    Serial.println("Failed to initialize ADS.");
  }

  LittleFS.begin();
  if (!LittleFS.begin(FORMAT_IF_FAILED)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  delay(100);

  // Файл с параметрами отжима
  if (LittleFS.exists("/sets.txt")) {
    File file = LittleFS.open("/sets.txt", "r");
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    file.close();
  } else {
    File file = LittleFS.open("/sets.txt", "w");
    file.print(factorySettings);
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    file.close();
  }
  // serializeJson(doc, Serial);

  Serial.println();
  delay(100);

  // // Файл с настройками
  if (LittleFS.exists("/params.txt")) {
    File f = LittleFS.open("/params.txt", "r");
    DeserializationError error = deserializeJson(sok, f);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    f.close();
  } else {
    File f = LittleFS.open("/params.txt", "w");
    f.print(facttoryParams);
    DeserializationError error = deserializeJson(sok, f);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    f.close();
  }

  Serial.println();

  // tftMainScreen();

  serializeJson(doc, jsonString);
  serializeJson(sok, jsonParams);
  serializeJson(sok, Serial);
  Serial.println("");


   if (sok[0]["beeper"].as <bool>()) {
    tone(BUZZER, 2000, 100);
  }

   wifiConnectScreen();

  // disp.printRight(true);  
  // disp.setCursorEnd();  
  // disp.brightness(3);
  display.setBrightness(BRIGHT_7);
  

  ElegantOTA.begin(&server); 
  // // ElegantOTA callbacks
  // ElegantOTA.onStart(onOTAStart);
  // ElegantOTA.onProgress(onOTAProgress);
  // ElegantOTA.onEnd(onOTAEnd);


  String ssidWeb = sok[0]["ssid"].as<String>();
  String passWeb = sok[0]["password"].as<String>();

  WiFi.begin(ssidWeb, passWeb);
  Serial.print("Подключение к WiFi");

  // Ожидаем подключения
  for (int i = 0; i < 5; i++) { // Ожидание 5 секунд
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
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Не удалось подключиться к WiFi, создаем точку доступа...");
    WiFi.disconnect(true);
    delay(300);
    
    // Настройка точки доступа
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid, password);
    // WiFi.softAPConfig(localIP, localIP, subnet); // Установка статического IP для AP

    Serial.print("Точка доступа создала IP-адрес: ");
    Serial.println(WiFi.softAPIP());
    isConnectedToRouter = false;
  }
   
   if (isConnectedToRouter) {
    wifiScreenRouter();
   } else {
    wifiScreenAP();
   }
    
  //  tftMainScreen();
  
  server.begin();

  Serial.println("mDNS responder started");
  // Initialize mDNS
  if (!MDNS.begin(dnsName)) { // Set the hostname to "esp32.local"
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  initWebSocket();

  // Если просто зашли на адрес контроллера, пушится всё что в SendHTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request -> send(200, "text/html", SendHTML());
  });

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


}

void loop() {

  if (ADS.isBusy() == false)
  {
    val_0 = ADS.getValue();
    //  request a new one
    ADS.requestADC(0);
    // Serial.print("\tAnalog0: ");
    // Serial.println(val_0);
    constrain(ADS.getValue()/pressureDivivder,0,1000);

  }

  if (saveSetsFlag) {
    saveSets();
    saveSetsFlag = false;
  }

  // disp.tick();

  ElegantOTA.loop();

  // находим длину массива выбранной культуры
  arrayLen = doc[chozenSeed]["value"].size();

  // находим последний элемент в массиве, он же время окончания (в секундах)
  endTime = doc[chozenSeed]["value"][arrayLen - 1].as<int>() * 60;

  // находим оставшееся время процесса отжатия
  timeLeft = (endTime - totalTime);

  // переменные для подсчёта оставшегося времени
  timeLeftHour = timeLeft / 3600;
  timeLeftMins = (timeLeft - timeLeftHour * 3600) / 60;
  timeLeftSec = timeLeft % 60;

  // длительность программы в часах и минутах
  totalHour = endTime / 3600;
  totalMins = (endTime - totalHour * 3600) / 60;

  // переменные для подсчёта прошедшего времени
  pastHours = totalTime / 3600;
  pastMins = totalTime / 60 - totalTime / 3600 * 60;
  pastSec = totalTime % 60;

  // server.handleClient();

  safetyTime = sok[0]["protection"].as <int> ();
  beeperFlag = sok[0]["beeper"].as <bool> ();
  sensorPressure = sok[0]["sensor"].as <int> ();



  if (timerIndicatorDelay.isReady()) {

    // disp.clear();
    // disp.print(pressure);
    // disp.update();
    display.showNumber(pressure);


  }

  // Коэффициены могут незначительно менять в зависимости от делителя
  switch (sensorPressure) {
  case 60:
    pressureDivivder = PRESSURE60;
    break;

  case 70:
    pressureDivivder = PRESSURE70;
    break;

  case 80:
    pressureDivivder = PRESSURE80;
    break;

  default:
    break;
  }

  eb.tick();
  startBtn.tick();
  stopBtn.tick();
  stopLed.tick();


  /*
Прокрутка культур или диапазонов
  */

 /* 1-mainScreen, 2-diapazonScreen, 3-chartScreen, 4-processScreen, 
5-alarmScreen, 6-endScreen, 7-WiFi connect screen, 8-WiFi info screen 
*/
  if (eb.left() && !wasStartedFlag) {
    switch (currentScreen) {
    case 1: // если на экране с культурами
      pointer = constrain(pointer - 1, 0, doc.size() - 1);
      if (doc.size() > PROGS_ON_SCREEN && (pointer+1)%PROGS_ON_SCREEN == 0) {
        cultureScreenNuber = constrain(cultureScreenNuber - 1, 1, cultureScreens);
        firstCulture = constrain(firstCulture - PROGS_ON_SCREEN, 0, (cultureScreens-1)*PROGS_ON_SCREEN);
        mainScreenUpdate();
      }
      chozenSeed = pointer;
      cursorBack();
      break;

    case 2: // если на экране с диапазонами
      if (arrayLen >= 23 && diapScreenNumber != 1) {
        diapScreenNumber = constrain(diapScreenNumber - 1, 1, diapScreens);
        firstDiap = constrain(firstDiap - 8, 0, (diapScreens-1)*8);
        diapazonsForParams();
      }
      break;
    case 6:
    case 8:
      currentScreen = 1;
      tftMainScreen();
    default:
      break;
    }
  }

  if (eb.right()) {
    switch (currentScreen) {
    case 1: // если на экране с культурами
      pointer = constrain(pointer + 1, 0, doc.size() - 1);
      if (doc.size() > PROGS_ON_SCREEN && pointer%PROGS_ON_SCREEN == 0) {
        cultureScreenNuber = constrain(cultureScreenNuber + 1, 1, cultureScreens);
        firstCulture = constrain(firstCulture + PROGS_ON_SCREEN, 0, (cultureScreens-1)*PROGS_ON_SCREEN);
        mainScreenUpdate();
      }
      chozenSeed = pointer;
      cursorForward();
      break;
    case 2: // если на экране с диапазонами
      if (arrayLen > 24 && diapScreenNumber != diapScreens) {
        diapScreenNumber = constrain(diapScreenNumber + 1, 1, diapScreens);
        firstDiap = constrain(firstDiap + 8, 0, (diapScreens-1)*8);
        diapazonsForParams();
      }
      break;
    case 6:
    case 8:
      currentScreen = 1;
      tftMainScreen();
    default:
      break;
    }
  }

  // Переходы с главного экрана к диапазонам по клику энкодера

  if (eb.click() && !wasStartedFlag) {
    switch (currentScreen) {
    case 1: // если на главном экране, переходим на диапазоны
      currentScreen = 2;
      diapazonScreen();
      chozenSeed = pointer;
      break;
    case 2: // если на диапазонах, переходим на чарт
      currentScreen = 3;
      chartScreen();
      break;
    case 3:
    case 8: // если экран с чартом или WiFi, переходим на главный
      currentScreen = 1;
      tftMainScreen();
      break;

    default:
      break;
    }
  }
/* 1-mainScreen, 2-diapazonScreen, 3-chartScreen, 4-processScreen, 
5-alarmScreen, 6-endScreen, 7-WiFi connect screen, 8-WiFi info screen 
*/
  // кнопка старта процесса
  if (startBtn.click() && !wasStartedFlag) { // не сработает, если находимся на аварийном экране или экране окончания
    switch (currentScreen) {
    case 8: // если экран с WiFi, переходим на главный
      currentScreen = 1;
      tftMainScreen();
      break; 
    case 5:
    case 6:
      break;
      // запускает процесс на других экранах
    default:
      stopLed.stop();
      chozenSeed = pointer;
      currentScreen = 4;
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
  if (currentScreen == 5) {
    if (stopBtn.click()) {
      stopLed.stop();
      tftMainScreen();
      currentScreen = 1;
    }
  }

  // если после окончания процесса нажать кнопку "стоп", ТО прекатится мигание stopLed
  if (stopBtn.press() && endFlag) {
    stopLed.stop();
  }

  /* если нажать на кнопку "старт", то поднимается флаг wasStartedFlag, время начинает бежать
  с нуля, запускается цикл назначения диапазона давлений
  */
  if (wasStartedFlag) totalTime = millis() / 1000ul - timeBeforeStart + continueTime;

  // главный алгоритм, поддерживающий давление в диапазоне minPress-maxPress
  if (pressure >= maxPress && isFilled) {
    pump_off();
    isFilled = 0;
  } else if (pressure <= minPress && isFilled == 0 && wasStartedFlag) {
    pump_on();
    isFilled = 1;
  }

  // проходимся по массиву культуры и меняем диапазон давлений по времени

  for (int i = 0; i < arrayLen - 3; i = i + 3) {
    if (totalTime < doc[chozenSeed]["value"][2].as<int>() * 60) {
      maxPress = doc[chozenSeed]["value"][0].as<int>();
      minPress = doc[chozenSeed]["value"][1].as<int>();
    } else if (totalTime >= doc[chozenSeed]["value"][i + 2].as<int>() * 60) {
      maxPress = doc[chozenSeed]["value"][i + 3];
      minPress = doc[chozenSeed]["value"][i + 4];
    }
  }
  // запуск процесса с текущего давления
  if (startBtn.hold() && !wasStartedFlag) {
    switch (currentScreen) { // не сработает для аварийного экрана и экрана окончания
    case 5:
    case 6:
      break;

    default:
      continueFlag = 1;
      break;
    }

  }
  /* Проходимся по массиву культуры, прверяем давление. Если выше нижнего значения диапазона,
  то устанавливаем инкремент к общему времени на значение, соответствующее предыдущему
  диапазону. Начинаем с 3-го диапазона, раньше не имеет смысла. Таким образом, если
  поднят флаг continueFlag, запустится процесс не с начала, а со времени, соответствующему
  диапазону давлений.
  */
  for (int i = 0; i < arrayLen - 1; i = i + 3) {
    if (doc[chozenSeed]["value"][i + 7].as<int>() > pressure && continueFlag == 1) {
      continueTime = doc[chozenSeed]["value"][i + 2].as<int>() * 60;
      continueFlag = 0;
      startProcess();
      stopLed.stop();
      currentScreen = 4;
    }
  }

  // отрубаем пресс по времени окончания
  if (totalTime >= endTime && wasStartedFlag) {
    stopProcess();
    stopLed.blink(100, 200, 600);
    endScreen();
    // totalTime = 0;
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

  if (stopBtn.step(2)) {
    resetWiFi();
    ESP.restart();
  } 

// вывод информации на экране процесса
  if (currentScreen == 4) {
    if (processUpdTmr.isReady()) {
      screenBeginFlag = true;
      tft.fillRect(20,110,32,30,TFT_BLACK);
      tft.setCursor(timeLeftHour > 9 ? 21 : 38, 113);
      tft.print(timeLeftHour);

      tft.fillRect(84, 110, 35, 35, TFT_BLACK);
      tft.setCursor(timeLeftMins > 9 ? 84 : 99, 113);
      tft.print(timeLeftMins);

      tft.fillRect(150, 110, 35, 35, TFT_BLACK);
      tft.setCursor(timeLeftSec > 9 ? 150 : 165, 113);
      tft.print(timeLeftSec);

      // tft.fillRect(20,182,180,35,TFT_BLACK);
      tft.setTextColor(TFT_ORANGE, TFT_BLACK);
      tft.setCursor(40, 185);
      tft.print(maxPress);
      tft.print(" - ");
      tft.print(minPress);

    }
  }

  // Вывод в Serial
  if (timerSerialDelay.isReady() && DEBUG == 1) {

    // Serial.println("========");
    // Serial.print("End Time: ");
    // Serial.println(endTime);
    // Serial.println(pointer);
    // Serial.print("Культура: ");
    // Serial.println(doc[chozenSeed]["name"].as<const char *>());
    // Serial.print("CultOnScreen: ");
    // Serial.println(cultOnScreen);
    // Serial.print("Screen numbre: ");
    // Serial.println(currentScreen);

    // Serial.print("Max-min press: ");
    // Serial.print(maxPress);
    // Serial.print("-");
    // Serial.println(minPress);
    // Serial.print("ADC out: ");

        Serial.print("ADC Out");
     Serial.println(ADS.getValue());

             Serial.print("Safety time");
     Serial.println(safetyTime);
                 Serial.print("Total time");
     Serial.println(totalTime);
             Serial.print("pressure");
     Serial.println(pressure);

                  Serial.print("Стоптаймер готов?");
     Serial.println(pumpOnTmr.isReady());


  }
  ws.cleanupClients();

  if (webStartFlag) {
    startProcess();
    webStartFlag = false;
  }
}