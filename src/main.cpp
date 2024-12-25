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
#include "Update.h"
#include <ElegantOTA.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <TFT_eSPI.h>
extern TFT_eSPI tft; 
TFT_eSPI tft = TFT_eSPI();

extern TFT_eSprite mySprite;
extern TFT_eSprite signalSprite;

TFT_eSprite mySprite = TFT_eSprite(&tft);
TFT_eSprite signalSprite = TFT_eSprite(&tft);

#include <ESPmDNS.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include <string.h>

#include <Adafruit_MCP23X17.h>
Adafruit_MCP23X17 mcp;

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

  if (!mcp.begin_I2C()) {
    Serial.println("Error mcp start!");
    while (1);
  }

  mcp.pinMode(PUMP_LED, OUTPUT);
  mcp.pinMode(HEAT_LED, OUTPUT);
  mcp.pinMode(PUMP_ZERO, OUTPUT);
  mcp.pinMode(BLINK_LED, OUTPUT); 

  // mcp.pinMode(START_BUTTON, INPUT_PULLUP);
  // mcp.pinMode(STOP_BUTTON, INPUT_PULLUP);

  pinMode(STARTBUTTON, INPUT_PULLUP);
  pinMode(STOPBUTTON, INPUT_PULLUP);
  

  mcp.digitalWrite(PUMP_LED, LOW);
  mcp.digitalWrite(HEAT_LED, LOW);
  mcp.digitalWrite(PUMP_ZERO, LOW);
  mcp.digitalWrite(BLINK_LED, LOW);


  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  mySprite.createSprite(200, 40); 
  mySprite.setTextColor(TFT_ORANGE, TFT_BLACK); // Установка цветов текста и фона

  signalSprite.createSprite(100, 30);

  enc.setEncReverse(0);
  enc.counter = 0; // сбросить счётчик энкодера

  timerSerialDelay.setInterval(500);
  timerIndicatorDelay.setInterval(200);
  processUpdTmr.setInterval(200);
  btnStatusCheck.setInterval(50);
  wifiStrengthTmr.setInterval(500);

  // Подключение внешнего АЦП
  checkADCConnection();

  if (isADCConnected) {
  Serial.println("ADS connected :-)");
  ADS.setGain(1);

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
  display2.setBrightness(BRIGHT_7);
  

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

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(204);
});

  // server.on("/startform", HTTP_GET, [](AsyncWebServerRequest * request) {
  //   handleStartForm(request);
  // });
  

  server.on("/web-stop", HTTP_GET, [](AsyncWebServerRequest * request) {
    handleWebStop(request);
  });

  server.on("/web-start", HTTP_GET, [](AsyncWebServerRequest *request)
            { handleWebStart(request); });


    myTime.end = 0;
    myTime.totalHour = 0;
    myTime.leftMins = 0;
    myTime.past.hours = 0;
    myTime.past.mins = 0;
    myTime.past.sec = 0;
    myTime.maintainStart = 0;
}

void loop() {

  // checkButtons();
  enc.tick();
  startBtn.tick();
  stopBtn.tick();
  blinker.tick();

  // Обработка нажатия кнопки "Старт"

  if (startBtn.click() && !wasStartedFlag) { 
  Serial.println("Кнопка 'Старт' нажата");
  switch (currentScreen) {
  case WIFIINFO: // если экран с WiFi, переходим на главный
    currentScreen = MAIN;
    tftMainScreen();
    break; 
  case ALARM:
  case END:
  case PRE_HEAT:
  case PROCESS:
    break;
    // запускает процесс на других экранах
  default:
    blinker.stopBlink(); // останавливаем мигание
    startHeating(); // запускаем разогрев
    break;
  }
  }

  // if (startButtonPressed) {
  //   startButtonPressed = false; // Сбрасываем флаг после обработки
  //   onStartButtonPressed();
  // }

  // Обработка нажатия кнопки "Стоп"
  // if (stopButtonPressed) {
  //   stopButtonPressed = false; // Сбрасываем флаг после обработки
  //   onStopButtonPressed();
  // }

  if (stopBtn.click()) {
    switch (currentScreen)
    {
    case ALARM:
    case END:
      blinker.stopBlink();
      tftMainScreen();
      currentScreen = MAIN;
      break;
    default:
      if (wasStartedFlag || preHeatStage) {
      stopProcess();
      endScreen();
    }
      break;
    }
  }
  
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

  if (timerIndicatorDelay.isReady()) {
    display.showNumber(sensor.pressure);
    display2.showNumber(sensor.temp);
  } 


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
  
  case 100:
    pressureDivider = PRESSURE100;
    break;

  default:
  pressureDivider = 1.0; // Значение по умолчанию
    break;
  }

  


  /*
Прокрутка культур или диапазонов
  */
    if (enc.left() && !wasStartedFlag) {
    encRotate(true);
    Serial.println("Left");
    Serial.print("Программа: ");
    Serial.println(doc[chozenSeed]["name"].as<const char *>());
  } 

    if (enc.right()) {
    encRotate(false);
    Serial.println("Right");
    Serial.print("Программа: ");
    Serial.println(doc[chozenSeed]["name"].as<const char *>());
  } 

  //Переходы с главного экрана к диапазонам по клику энкодера

  // if (enc.isPress() && !wasStartedFlag) {
  if (enc.click() && !wasStartedFlag) {
    encClick();
  }



  // если находимся на аварийном экране, то кнопка "стоп" переводит на главный экран
  // if (currentScreen == ALARM) {
  //   if (stopBtn.click()) {
  //     // stopLed.stop();
  //     blinker.stopBlink();
  //     tftMainScreen();
  //     currentScreen = MAIN;
  //   }
  // }

  // если после окончания процесса нажать кнопку "стоп", ТО прекатится мигание stopLed
  // if (stopBtn.press() && endFlag) {
  //   // stopLed.stop();
  //   blinker.stopBlink();
  // }

  /* если нажать на кнопку "старт", то поднимается флаг wasStartedFlag, время начинает бежать
  с нуля, запускается цикл назначения диапазона давлений
  */
  if (wasStartedFlag) myTime.current = millis() / 1000ul - myTime.beforeStart + continueTime;

  // алгоритм сброса давления
if (sensor.maxPress == 0 && sensor.minPress == 0 && wasStartedFlag) {
    // Если оба значения давления равны нулю
    pump_off();  // Выключаем основную помпу
    sensor.isFilled = 0;
    
    if (!pumpSwitchTmr.isEnabled()) {
        pumpSwitchTmr.setTimeout(1000);  // Устанавливаем задержку 1 с
    }
    
    if (pumpSwitchTmr.isReady()) {  // Ждем истечения таймера
        if (sensor.pressure > 2) {
            mcp.digitalWrite(PUMP_ZERO, HIGH);  // Включаем вторую помпу
        } else {
            mcp.digitalWrite(PUMP_ZERO, LOW);   // Выключаем вторую помпу
        }
    }
} else {
    // Стандартная логика контроля давления
    if (sensor.pressure >= sensor.maxPress && sensor.isFilled) {
        pump_off();
        sensor.isFilled = 0;
    } else if (sensor.pressure <= sensor.minPress && !sensor.isFilled && wasStartedFlag) {
        if(sensor.minPress != 0) {
            pump_on();
            sensor.isFilled = 1;
        }
    }
    mcp.digitalWrite(PUMP_ZERO, LOW);  // Убеждаемся что вторая помпа выключена
    pumpSwitchTmr.stop();  // Останавливаем таймер
}

  if (sensor.temp >= sensor.maxTemp && sensor.isWarmed) {
    heat_off();
    sensor.isWarmed = 0;
  } else if (sensor.temp <= sensor.minTemp && !sensor.isWarmed && wasStartedFlag) {
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
    blinkHundredTimes(); 
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
      // stopLed.blink(100, 200, 600);
      blinkHundredTimes(); 
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
  // if (stopBtn.step(2)) {
  //   resetWiFi();
  //   ESP.restart();
  // } 

// вывод информации на экране процесса
  if (currentScreen == PROCESS) {
    if (processUpdTmr.isReady()) {
      screenBeginFlag = true;
      updateDisplays();
    }
  }

  if (currentScreen == PRE_HEAT) {
    updatePreHeat();
  }

  if (currentScreen == WIFIINFO && wifiStrengthTmr.isReady()) {
    // tft.fillRect(155, 15, 60, 30, TFT_BLACK);
    updateSignalStrength();

  }

if (tempTimer.isReady()) sensor.temp = constrain(thermocouple.readCelsius(), 0, 1000);

  // Вывод в Serial
  if (timerSerialDelay.isReady() && DEBUG == 1) {

    Serial.println("========");

    // Serial.println(chozenSeed);
    Serial.print("Программа: ");
    Serial.println(doc[chozenSeed]["name"].as<const char *>());

    Serial.print("Max-min press: ");
    Serial.print(sensor.maxPress);
    Serial.print("-");
    Serial.println(sensor.minPress);
    Serial.print("Max-min temp: ");
    Serial.print(sensor.maxTemp);
    Serial.print("-");
    Serial.println(sensor.minTemp);
    Serial.print("preHeatStage: ");
    Serial.println(preHeatStage);


  }
  ws.cleanupClients();

  if (webStartFlag) {
    startProcess();
    webStartFlag = false;
  }

  checkPreHeat();

  if (preHeatStage) pressureControlPreHeatMode();

}