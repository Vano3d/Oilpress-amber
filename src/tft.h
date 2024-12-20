#include <Arduino.h>


void some(String name, String eee) {
  doc[chozenSeed]["stages"][eee][name].as<int>();
}

// вывод диапазонов

void printDiapazons() {
    for (int i = firstDiap; i < firstDiap + 2 && i < allDiaps; i++) {
        // Разделитель
        tft.println(F("-----"));
        
        // Вывод давления
        tft.print(doc[chozenSeed]["stages"][i]["maxPress"].as<int>());
        tft.print(F("-"));
        tft.print(doc[chozenSeed]["stages"][i]["minPress"].as<int>());
        tft.println(F(" бар"));
        
        // Вывод температуры
        tft.print(doc[chozenSeed]["stages"][i]["maxTemp"].as<int>());
        tft.print(F("-"));
        tft.print(doc[chozenSeed]["stages"][i]["minTemp"].as<int>());
        tft.println(F(" градусов"));
        
        // Вывод времени
        tft.print(doc[chozenSeed]["stages"][i]["time"].as<int>());
        tft.println(F(" мин"));
    }
}

// стрелочка вверх-вниз под диапазонами
void drwawArrows() {
    tft.unloadFont();
    if (allDiaps > 3) {
        // Стрелка вниз
    if (diapScreenNumber != diapScreens) {
      tft.drawLine(100, 300, 120, 310, TFT_WHITE);
      tft.drawLine(120, 310, 140, 300, TFT_WHITE);
    } else {
      tft.drawLine(100, 310, 120, 300, TFT_WHITE);
      tft.drawLine(120, 300, 140, 310, TFT_WHITE);
    }
    }
}

// вывод диапазонов
void diapazonsForParams() {
    tft.fillRect(0, 96, 240, 300, TFT_BLACK);
    tft.setCursor(0, 100);
    tft.loadFont(myFont24);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    
    allDiaps = doc[chozenSeed]["stages"].size(); // Предполагается, что это количество стадий
    
    // Находим количество экранов с диапазонами (по 3 диапазона на экран)
    diapScreens = (allDiaps % 2 == 0) ? (allDiaps / 2) : (allDiaps / 2 + 1);
    
    printDiapazons();
    drwawArrows();
}

void nameAndTime() {
myTime.end = 0;

myTime.end = mySeed.calcEndTime();
myTime.totalHour = myTime.end / 3600;
myTime.leftMins = (myTime.end - myTime.totalHour * 3600) / 60;  

Serial.print("Name: ");
Serial.println(mySeed.name());
Serial.print("End Time: ");
Serial.println(myTime.end);

  tft.fillScreen(TFT_BLACK);
  tft.loadFont(myFont28);
  tft.setCursor(5, 5);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(mySeed.name());
  tft.unloadFont();

  tft.loadFont(myFont24);
  tft.setCursor(5, 38);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.print("всего ");

  if (myTime.totalHour != 0) {

    tft.print(myTime.totalHour);
    tft.print(F(" ч "));
  }
  if (myTime.leftMins != 0) {

    tft.print(myTime.leftMins);
    tft.print(F(" м"));
  }
}


void mainScreenUpdate() {
    tft.fillRect(1, 35, 239, 300, TFT_BLACK);
    tft.drawString(">", 2, chozenSeed%PROGS_ON_SCREEN * 30 + 39);
  
  // находим кол-во экранов с культурами
  if (doc.size()%PROGS_ON_SCREEN == 0) {
    cultureScreens = doc.size()/PROGS_ON_SCREEN;
  } else {
    cultureScreens = doc.size()/PROGS_ON_SCREEN + 1;
  }
  
  // вычисляем кол-во культур на экране
  if (doc.size() > PROGS_ON_SCREEN) {
    if (doc.size()%PROGS_ON_SCREEN != 0) {
      if (cultureScreenNuber == cultureScreens) {
        cultOnScreen = doc.size()%PROGS_ON_SCREEN;
      } else cultOnScreen = PROGS_ON_SCREEN;
    } else cultOnScreen = PROGS_ON_SCREEN;
  } else cultOnScreen = doc.size();

  // вывод культур циклом
  for (int i = 0; i < cultOnScreen; i++) {
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawString(doc[i + firstCulture]["name"].as<String>(), 20, 40 + i*30);
  }
}
//вывод меню выбора культуры
void tftMainScreen() {
  currentScreen = MAIN;
  tft.fillScreen(TFT_BLACK);
  diapScreenNumber = 1;
  firstDiap = 0;
  tft.unloadFont();
  
  tft.fillRect(1, 35, 239, 300, TFT_BLACK);
  
  tft.loadFont(myFont28);
  tft.setCursor(0, 3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Программы");
  tft.unloadFont();
  tft.loadFont(myFont28);

  mainScreenUpdate();
}

// Экран с временим отжима и диапазонами
void diapazonScreen() {
  nameAndTime();
  tft.setCursor(5, 70);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Диапазоны:");
  tft.unloadFont();
  diapazonsForParams();
}


// Экран с процессом
void processScreen() {
  nameAndTime();

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(65, 82);
  tft.println("Осталось");
  textWidth = tft.textWidth("Диапазон, бар");
  tft.setCursor((tft.width() - textWidth) / 2, 155);
  tft.println("Диапазон, бар");

  textWidth = tft.textWidth("Диапазон, град");
  tft.setCursor((tft.width() - textWidth) / 2, 230);
  tft.println("Диапазон, град");
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  screenBeginFlag = false;
}

// экран окончания процесса
void endScreen()
{
  currentScreen = END;
  tft.fillScreen(TFT_BLACK);
  tft.loadFont(myFont28);
  tft.setCursor(5, 5);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  String endName = doc[chozenSeed]["name"].as<String>();
  textWidth = tft.textWidth(endName);
  tft.setCursor((tft.width() - textWidth)/2, 10);
  tft.println(endName);
  tft.unloadFont();

  tft.loadFont(myFont28);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  textWidth = tft.textWidth("ПРОГРАММА");
  tft.setCursor((tft.width() - textWidth)/2, 80);
  tft.println("ПРОГРАММА");
  textWidth = tft.textWidth("ЗАКОНЧЕНА");
  tft.setCursor((tft.width() - textWidth)/2, 115);
  tft.println("ЗАКОНЧЕНА");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 170);
  tft.print("за  ");
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.print(myTime.past.hours);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(" ч  ");
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.print(myTime.past.mins);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(" м  ");
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.print(myTime.past.sec);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(" с");
  tft.unloadFont();
}

// аварийный экран
void alarmScreen()
{
  currentScreen = ALARM;
  tft.fillScreen(TFT_BLACK);
  tft.loadFont(myFont28);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  textWidth = tft.textWidth("АВАРИЯ!");
  tft.setCursor((tft.width() - textWidth)/2, 40);
  tft.println("АВАРИЯ!");
  tft.unloadFont();
  tft.loadFont(myFont24);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setCursor(0, 100);
  tft.println("Проверьте рукоятку");
  tft.setCursor(0, 150);
  tft.println("Проверьте");
  tft.setCursor(0, 175);
  tft.println("гидравлику");
  tft.setCursor(0, 220);
  tft.println("Перезапустите про-");
  tft.setCursor(0, 245);
  tft.println("грамму");
  tft.unloadFont();
}

void cursorForward() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(">", 2, chozenSeed%PROGS_ON_SCREEN * 30 + 39);
  if (chozenSeed%PROGS_ON_SCREEN != 0) tft.fillRect(2, chozenSeed%PROGS_ON_SCREEN * 30 + 12, 15, 17, TFT_BLACK);
}

void cursorBack() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(">", 2, chozenSeed%PROGS_ON_SCREEN * 30 + 39);
  tft.fillRect(2, (chozenSeed%PROGS_ON_SCREEN + 1) * 30 + 42, 15, 17, TFT_BLACK);
}

void displaySignalStrength() {
  // Учитываем масштабы RSSI
  int level = map(WiFi.RSSI(), -100, -30, 0, 5); // Преобразование в уровень (0-5)

  // Отображаем столбики
  int barHeightMin = 9; // мин. высота одного столбика
  int spacing = 12; // Расстояние между столбиками
  byte barWidth = 7; // Ширина столбика

  for (int i = 0; i < 5; i++) {
    int x = i * spacing; // Позиция начала столбика по X в спрайте
    int y = 20; // Позиция начала столбика по Y

    // Определяем цвет в зависимости от заполнения
    if (i < level) {
      signalSprite.fillRect(x, y - ((i + 1) * 3), barWidth, barHeightMin + (i * 3), TFT_GREEN); // Заполненный зеленый цвет
    } else {
      signalSprite.drawRect(x, y - ((i + 1) * 3), barWidth, barHeightMin + (i * 3), TFT_GREEN); // Пустой цвет (контур)
    }
  }
}

void updateSignalStrength() {
  // Очищаем спрайт перед обновлением
  signalSprite.fillSprite(TFT_BLACK);
  
  // Получаем уровень сигнала и отображаем его
  displaySignalStrength();
  
  // Отправляем спрайт на экран
  signalSprite.pushSprite(160, 15); // X и Y позиции на основном экране
}

void wifiConnectScreen() {
  currentScreen = WIFICONNECT;
  tft.fillScreen(TFT_BLACK);
  tft.loadFont(myFont28);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  textWidth = tft.textWidth("WiFi");
  tft.setCursor((tft.width() - textWidth)/2, 20);
  tft.println("WiFi");
  tft.unloadFont();

  tft.loadFont(myFont24);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  textWidth = tft.textWidth("подключение...");
  tft.setCursor((tft.width() - textWidth)/2, 145);
  tft.println("подключение...");
  tft.unloadFont();
}

void wifiScreenRouter() {
  currentScreen = WIFIINFO;
  tft.fillScreen(TFT_BLACK);
  tft.loadFont(myFont28);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  textWidth = tft.textWidth("WiFi");
  tft.setCursor((tft.width() - textWidth)/2, 20);
  tft.println("WiFi");
  tft.unloadFont();

  tft.loadFont(myFont24);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  textWidth = tft.textWidth("Подключено");
  tft.setCursor((tft.width() - textWidth)/2, 66);
  tft.println("Подключено");

  textWidth = tft.textWidth("к роутеру");
  tft.setCursor((tft.width() - textWidth)/2, 100);
  tft.println("к роутеру");

  textWidth = tft.textWidth("Введите адрес");
  tft.setCursor((tft.width() - textWidth)/2, 145);
  tft.println("Введите адрес");

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  textWidth = tft.textWidth("192.168.1.137");
  tft.setCursor((tft.width() - textWidth)/2, 185);
  tft.println(WiFi.localIP());

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  textWidth = tft.textWidth("или");
  tft.setCursor((tft.width() - textWidth)/2, 220);
  tft.println("или");

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  textWidth = tft.textWidth("maslobot.local");
  tft.setCursor((tft.width() - textWidth)/2, 257);
  tft.println(dnsName + ".local");

  tft.unloadFont();
}

void wifiScreenAP() {
  currentScreen = WIFIINFO;
  tft.fillScreen(TFT_BLACK);
  tft.loadFont(myFont28);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  textWidth = tft.textWidth("WiFi");
  tft.setCursor((tft.width() - textWidth)/2, 20);
  tft.println("WiFi");
  tft.unloadFont();

  tft.loadFont(myFont24);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  textWidth = tft.textWidth("точка доступа");
  tft.setCursor((tft.width() - textWidth)/2, 55);
  tft.println("точка доступа");

  tft.setCursor(5, 110);
  tft.print("сеть ");

   tft.setCursor(5, 113);
   tft.setTextColor(TFT_ORANGE, TFT_BLACK);
   tft.setCursor(77, 113);
   tft.print(ssid);

   tft.setCursor(5, 145); 
   tft.setTextColor(TFT_WHITE, TFT_BLACK);
   tft.print("пароль ");
   
   tft.setCursor(100, 145);
   tft.setTextColor(TFT_ORANGE, TFT_BLACK);
   tft.print(password);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  textWidth = tft.textWidth("Введите адрес");
  tft.setCursor((tft.width() - textWidth)/2, 194);
  tft.println("Введите адрес:");

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  textWidth = tft.textWidth("192.168.4.1");
  tft.setCursor((tft.width() - textWidth)/2, 228);
  tft.println("192.168.4.1");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  textWidth = tft.textWidth("или");
  tft.setCursor((tft.width() - textWidth)/2, 257);
  tft.println("или");

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  textWidth = tft.textWidth("maslobot.local");
  tft.setCursor((tft.width() - textWidth)/2, 288);
  tft.println(dnsName + ".local");
  tft.unloadFont();
}

void updateDisplays() {
    mySprite.loadFont(myFont28);
    // Обновление времени

    mySprite.fillSprite(TFT_BLACK); // Очистка спрайта
    mySprite.setCursor(0, 0);
    mySprite.print(String(myTime.leftHour) + " ч " + String(myTime.leftMins) + " м " + String(myTime.leftSec)+ " с");
    mySprite.pushSprite(40, 113); // Позиция на дисплее

    // Обновление давления
    mySprite.fillSprite(TFT_BLACK); // Очистка спрайта
    mySprite.setCursor(0, 0);
    mySprite.print(String(sensor.maxPress) + " - " + String(sensor.minPress) + " (" + String(sensor.pressure) + ")");
    mySprite.pushSprite(40, 185); // Позиция на дисплее

    // Обновление температуры
    mySprite.fillSprite(TFT_BLACK); // Очистка спрайта
    mySprite.setCursor(0, 0);
    mySprite.print(String(sensor.maxTemp) + " - " + String(sensor.minTemp) + " (" + String(sensor.temp) + ")");
    mySprite.pushSprite(40, 265); // Позиция на дисплее

    mySprite.unloadFont();

    if (mcp.digitalRead(PUMP_LED)) {
      tft.fillCircle(20, 195, 5, TFT_GREEN);
    } else if (mcp.digitalRead(PUMP_ZERO)) {
      tft.fillCircle(20, 195, 5, TFT_BLUE);
    } else {
      tft.fillCircle(20, 195, 5, TFT_BLACK);
    }
 
    if (mcp.digitalRead(HEAT_LED)) {
      tft.fillCircle(20, 275, 5, TFT_GREEN);
    } else {
      tft.fillCircle(20, 275, 5, TFT_BLACK);
    }
    
}