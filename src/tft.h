#include <Arduino.h>


void some(String name, String eee) {
  doc[chozenSeed]["stages"][eee][name].as<int>();
}

// вывод диапазонов
void printDiapazons() {
    for (int i = firstDiap; i < firstDiap+3; i += 1) {
    if (1==1) {
      // if ((doc[chozenSeed]["value"][i].as <int> ()) !=NULL) {
      tft.print(doc[chozenSeed]["stages"][i]["maxPress"].as<int>());
      tft.print(F("-"));
      tft.print(doc[chozenSeed]["stages"][i]["minPress"].as<int>());
      tft.println(F(" бар "));
      tft.print(F(" "));
      tft.print(doc[chozenSeed]["stages"][i]["maxTemp"].as<int>());
      tft.print(F("-"));
      tft.print(doc[chozenSeed]["stages"][i]["minTemp"].as<int>());
      tft.println(F(" градусов"));
      tft.print(F(" "));
      tft.print(doc[chozenSeed]["stages"][i]["time"].as<int>());
      tft.println(F(" мин"));
    }
  }
}

// стрелочка вверх-вниз под диапазонами
void drwawArrows() {
    tft.unloadFont();
  if (allDiaps > 8) {
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
  allDiaps = arrayLen / 3; // кол-во всех диапазонов внутри культуры

// находим количество экранов с диапазонами
  if (allDiaps%8 == 0) {
    diapScreens = allDiaps/8;
  } else {
    diapScreens = allDiaps/8 + 1;
  }
  printDiapazons();
  drwawArrows();
}

// рисуем бочонок
void barrelOn() {
  tft.drawLine(CORDX, CORDY, CORDX, CORDY+40, TFT_WHITE); // бочка лево
  tft.drawLine(CORDX, CORDY+40, CORDX+40, CORDY+40, TFT_WHITE); // бочка низ
  tft.drawLine(CORDX+40, CORDY+40, CORDX+40, CORDY, TFT_WHITE); // бочка право

  tft.drawLine(CORDX+5, CORDY+13, CORDX+35, CORDY+13, TFT_WHITE); // поршень
  tft.drawLine(CORDX+12, CORDY+13, CORDX+12, CORDY-7, TFT_WHITE); // ГЦ лево
  tft.drawLine(CORDX+12, CORDY-7, CORDX+28, CORDY-7, TFT_WHITE); // ГЦ верх 
  tft.drawLine(CORDX+28, CORDY-7, CORDX+28, CORDY+13, TFT_WHITE); // ГЦ право 

  tft.drawLine(CORDX+20, CORDY+20, CORDX+20, CORDY+33, TFT_WHITE); // стрелка 
  tft.drawLine(CORDX+20, CORDY+33, CORDX+16, CORDY+27, TFT_WHITE); // стрелка лево 
  tft.drawLine(CORDX+20, CORDY+33, CORDX+24, CORDY+27, TFT_WHITE); // стрелка право 
}

void barrelOff() {
   tft.fillRect(CORDX-2, CORDY-9, CORDX+42, CORDY+42, TFT_BLACK);
}

// рисуем график
void plotChart() {
  tft.setCursor(0, 300);
  byte coordX;
  byte coordY;
  byte coordX2;
  byte coordY2;
  float coeffX = 240.0 / (float(myTime.end / 60));
  float coeffY = 120.0 / float(doc[chozenSeed]["value"][arrayLen - 3].as < int > ());

  for (int i = 0; i < arrayLen - 1; i = i + 3) {
    coordX = doc[chozenSeed]["value"][i + 2].as < int > () * coeffX - 1;

    byte j;
    i < arrayLen - 4 ? j = i + 3 : j = i;
    coordY = 240 - doc[chozenSeed]["value"][j].as < int > () * coeffY;
    coordX2 = doc[chozenSeed]["value"][i + 5].as < int > () * coeffX - 1;

    byte k;
    i == arrayLen - 6 ? k = i + 3 : k = i + 6;
    coordY2 = 240 - doc[chozenSeed]["value"][k].as < int > () * coeffY;

    tft.fillCircle(coordX, coordY, 2, TFT_WHITE);
    if (i < arrayLen - 5) tft.drawLine(coordX, coordY, coordX2, coordY2, TFT_WHITE);
  }
}

void nameAndTime() {
myTime.end = 0;
    myTime.totalHour = 0;
    myTime.leftMins = 0;

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
    

    
    Serial.print("Total hour: ");
    Serial.println(myTime.totalHour);
    Serial.print("Total mins: ");
    Serial.println(myTime.leftMins);

    
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
  currentScreen = 1;
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

// экран с графиком
void chartScreen() {
  nameAndTime();
  plotChart();
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

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(58, 115);
  tft.println("ч");
  tft.setCursor(123, 115);
  tft.println("м");
  tft.setCursor(188, 115);
  tft.println("с");
  tft.unloadFont();
  tft.setTextSize(3);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  screenBeginFlag = false;
}

// экран окончания процесса
void endScreen()
{
  currentScreen = 6;
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
  currentScreen = 5;
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

void wifiConnectScreen() {
  currentScreen = 7;
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
  currentScreen = 8;
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
  currentScreen = 8;
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