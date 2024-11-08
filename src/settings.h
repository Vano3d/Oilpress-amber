#include <Arduino.h>
#include "pins.h"
// Adc out 26344
#define PRESSURE60 44.1 // normal 43.4
#define PRESSURE70 37.7 // normal 38.1 
#define PRESSURE80 33 // normal 33
#define FORMAT_IF_FAILED 1
#define DEBUG 0
#define I2C_SDA 4
#define I2C_SCL 5

#define PROGS_ON_SCREEN 9 // количество программ на экране
#define DIAPS_ON_SCREEN 8 // количество диапазонов на экране

// координаты бочонка
#define CORDX 100
#define CORDY 240

float pressureDivivder;
byte safetyTime;
byte beeperFlag;
byte sensorPressure;

const char* ssid = "maslobot1";  
const char* password = "1234567890";

// Disp1637Colon disp(DIO, CLK); -- глючит чё-то

#include <TM1637TinyDisplay.h>
TM1637TinyDisplay display(CLK, DIO);

GTimer timerIndicatorDelay(MS); 
GTimer timerSerialDelay(MS);
GTimer timerProcess(MS);
GTimer oledTimeout(MS);
// GTimer stopLedTimer(MS);
GTimer endTimer(MS);
GTimer buzzTimer1(MS);
GTimer buzzTimer2(MS);
GTimer pumpOnTmr(MS);
GTimer processUpdTmr(MS);
uint32_t processScreenBegin;

#define EB_DEB_TIME 50      // таймаут гашения дребезга кнопки (кнопка)
#define EB_CLICK_TIME 500   // таймаут ожидания кликов (кнопка)
#define EB_HOLD_TIME 600    // таймаут удержания (кнопка)

EncButton eb(S1, S2, KEY);
Button stopBtn(STOPBUTTON);
Button startBtn(STARTBUTTON);

Blinker stopLed(STOPLED);

// Adafruit_ADS1115 ads;
ADS1115 ADS(0x48);

TFT_eSPI tft = TFT_eSPI();

long totalTime = 0;
long endTime;

bool isFilled = 0; // флаг достижения порогового значения цикла
bool endFlag = 0; // флаг окончания всего процесса
unsigned int pressure = 0; // давление
bool wasStartedFlag = 0; // начинался ли процесс

int maxPress;
int minPress;

byte chozenSeed = 0; // номер выбранной культуры (от 0 до ХХ)
bool flag = 0;
long timeBeforeStart = 0;
int arrayLen;
unsigned int timeLeft;
unsigned int timeLeftHour;
unsigned int timeLeftMins;
unsigned int timeLeftSec;

unsigned int totalHour;
unsigned int totalMins;

unsigned int pastHours;
unsigned int pastMins;
unsigned int pastSec;

byte cultOnScreen;

// // stop blink code
// int ledState = LOW;
bool stopLedFlag = 0;

/* 1-mainScreen, 2-diapazonScreen, 3-chartScreen, 4-processScreen, 
5-alarmScreen, 6-endScreen, 7-WiFi connect screen, 8-WiFi info screen 
*/
byte currentScreen = 1; // текущий экран 
uint16_t textWidth = 0;
uint16_t diapTextWidth = 0;
unsigned int allDiaps; // количество всех диапазонов
byte diapScreens; // экранов с диапазонами
byte diapScreenNumber = 1; // текущий номер экрана с диапазонами 
byte firstDiap = 0; // первый диапазон на экране с диапазонами

byte firstCulture; // первая культура на экране, зависит от того, на каком экране сейчас
byte cultureScreens; // количество экранов с культурами
byte cultureScreenNuber = 1; // текущий номер экрана с культурами 

byte diapazon = 0;
bool buzzFlag = 0;

bool continueFlag = 0;
unsigned int continueTime = 0;
int cultCount;
String factorySettings = "[{\"name\":\"Тыква\",\"value\":[70,30,1,120,80,4,170,130,7,220,180,10,270,230,15,320,280,18,370,330,22,420,380,25,460,430,30,510,480,33,560,530,35,600,580,120]},{\"name\":\"Конопля\",\"value\":[70,30,2,120,80,5,170,130,8,220,180,12,270,230,15,320,280,20,400,360,23,460,420,25,510,480,42,600,580,80]},{\"name\":\"Чёрный тмин\",\"value\":[100,70,1,210,170,3,300,260,6,400,360,15,500,470,25,600,580,180]},{\"name\":\"Лён\",\"value\":[60,30,2,110,70,3,160,120,5,210,170,7,300,260,10,400,360,15,500,460,20,550,520,25,600,580,45]},{\"name\":\"Подсолнух\",\"value\":[60,30,7,110,70,25,160,120,35,210,170,45,260,220,53,310,270,59,360,320,63,400,370,68,490,460,75,600,580,80]},{\"name\":\"Кедр\",\"value\":[60,20,2,110,70,8,160,120,20,210,170,24,260,220,27,320,280,30,400,360,32,450,430,34,510,480,40,600,580,50]},{\"name\":\"Кунжут\",\"value\":[60,30,3,110,70,13,160,120,18,210,170,23,260,220,28,310,270,32,360,320,36,410,370,40,450,420,45,500,470,50,600,580,65]},{\"name\":\"Расторопша\",\"value\":[150,110,2,200,160,5,250,210,8,300,260,10,360,320,12,400,370,14,460,430,17,500,480,25,600,580,120]},{\"name\":\"Грецкий\",\"value\":[60,20,2,110,70,5,160,120,15,210,170,20,260,220,25,310,270,30,360,320,35,410,370,38,450,420,41,500,470,45,570,530,60]},{\"name\":\"Кокос\",\"value\":[70,30,1,120,80,3,170,130,5,220,180,10,300,260,12,400,360,15,500,470,20,600,570,50]},{\"name\":\"Стандарт 40\",\"value\":[70,30,5,110,80,12,150,120,16,200,170,20,250,220,24,300,270,27,350,320,30,430,400,32,500,470,34,550,520,36,595,570,40]},{\"name\":\"Стандарт 60\",\"value\":[70,30,5,110,80,12,150,120,16,200,170,20,250,220,24,300,270,27,350,320,30,430,400,32,500,470,34,550,520,36,595,570,60]},{\"name\":\"Стандарт 80\",\"value\":[70,30,5,110,80,12,150,120,16,200,170,20,250,220,24,300,270,27,350,320,30,430,400,32,500,470,34,550,520,36,595,570,80]},{\"name\":\"Стандарт 120\",\"value\":[70,30,5,110,80,12,150,120,16,200,170,20,250,220,24,300,270,27,350,320,30,430,400,32,500,470,34,550,520,36,595,570,120]},{\"name\":\"Стандарт 180\",\"value\":[70,30,5,110,80,12,150,120,16,200,170,20,250,220,24,300,270,27,350,320,30,430,400,32,500,470,34,550,520,36,595,570,180]},{\"name\":\"Стандарт 240\",\"value\":[70,30,5,110,80,12,150,120,16,200,170,20,250,220,24,300,270,27,350,320,30,430,400,32,500,470,34,550,520,36,595,570,240]}]";

String facttoryParams = "[{\"protection\": 12,\"beeper\": false, \"sensor\": 60, \"ssid\": \"name\", \"password\": \"password\"}]";

bool isADCConnected; // подключён ли модуль АЦП
bool wasStartedStockUp;
bool screenBeginFlag;
bool webStartFlag;
bool saveSetsFlag;

String dnsName = "maslobot";
bool isConnectedToRouter;

String getSensorReadings(){
  pork["timeLeftHour"] = timeLeftHour;
  pork["timeLeftMins"] =  timeLeftMins;
  pork["timeLeftSec"] = timeLeftSec;
  pork["maxPress"] = maxPress;
  pork["minPress"] = minPress;
  pork["pastHours"] = pastHours;
  pork["pastMins"] = pastMins;
  pork["pastSec"] = pastSec;
  pork["pressure"] = pressure;
  
  String jsonWS;
  serializeJson(pork, jsonWS);
  return jsonWS;
}

int tableIndex = 0;
int rowIndex = 0;
