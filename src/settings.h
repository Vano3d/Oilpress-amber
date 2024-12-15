#include <Arduino.h>
#include "pins.h"
// Adc out 26344
#define PRESSURE60 44.1 // normal 43.4
#define PRESSURE70 37.7 // normal 38.1 
#define PRESSURE80 33 // normal 33
#define FORMAT_IF_FAILED 1
#define DEBUG 1
#define I2C_SDA 4
#define I2C_SCL 5

#define PROGS_ON_SCREEN 9 // количество программ на экране
#define DIAPS_ON_SCREEN 8 // количество диапазонов на экране

// координаты бочонка
#define CORDX 100
#define CORDY 240



float pressureDivider;
byte safetyTime, beeperFlag, sensorPressure;
// byte beeperFlag;
// byte sensorPressure;



const char* ssid = "maslobot1";  
const char* password = "1234567890";

#include <TM1637TinyDisplay.h>
TM1637TinyDisplay display(CLK, DIO);
TM1637TinyDisplay display2(CLK2, DIO2);

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
GTimer btnStatusCheck(MS);

uint32_t processScreenBegin;

#define EB_DEB_TIME 50      // таймаут гашения дребезга кнопки (кнопка)
#define EB_CLICK_TIME 500   // таймаут ожидания кликов (кнопка)
#define EB_HOLD_TIME 600    // таймаут удержания (кнопка)

EncButton enc(S1, S2, KEY);

// Blinker stopLed(STOPLED);

// Adafruit_ADS1115 ads;
ADS1115 ADS(0x48);

TFT_eSPI tft = TFT_eSPI();

TFT_eSprite mySprite = TFT_eSprite(&tft);

bool endFlag = 0; // флаг окончания всего процесса
bool wasStartedFlag = 0; // начинался ли процесс

struct pastTime {
  unsigned int hours, mins, sec;
};

struct allTheTime {
  long current, end, beforeStart;
  unsigned long totalHour, totalMins, left, leftHour, leftMins, leftSec, continueTime;
  unsigned long maintainStart;
  bool continueFlag;
  pastTime past;
};

allTheTime myTime;

struct mySensors {
  unsigned int pressure, temp;
  int maxPress, minPress, maxTemp, minTemp;
  bool isFilled, isWarmed = 0;
};

mySensors sensor;


byte chozenSeed = 0; // номер выбранной культуры (от 0 до ХХ)
bool flag = 0;
int arrayLen;

byte cultOnScreen;

bool stopLedFlag = 0;

/* 1-mainScreen, 2-diapazonScreen, 3-chartScreen, 4-processScreen, 
5-alarmScreen, 6-endScreen, 7-WiFi connect screen, 8-WiFi info screen 
*/

enum Screen {
  MAIN = 1,
  DIAPAZONS,
  CHART,
  PROCESS,
  ALARM,
  END, 
  WIFICONNECT,
  WIFIINFO
};

Screen currentScreen = MAIN;

// byte currentScreen = 1; // текущий экран 
uint16_t textWidth, diapTextWidth;
unsigned int allDiaps; // количество всех диапазонов
byte diapScreens; // экранов с диапазонами
byte diapScreenNumber = 1; // текущий номер экрана с диапазонами 
byte firstDiap = 0; // первый диапазон на экране с диапазонами

byte firstCulture; // первая культура на экране, зависит от того, на каком экране сейчас
byte cultureScreens; // количество экранов с культурами
byte cultureScreenNuber = 1; // текущий номер экрана с культурами 

byte diapazon = 0;
bool buzzFlag, continueFlag;

unsigned int continueTime = 0;
int cultCount;

String factorySettings = "[{\"name\":\"Программа 1\",\"stages\":[{\"maxPress\":0,\"minPress\":0,\"maxTemp\":190,\"minTemp\":180,\"time\":10},{\"maxPress\":400,\"minPress\":390,\"maxTemp\":200,\"minTemp\":190,\"time\":10},{\"maxPress\":0,\"minPress\":0,\"maxTemp\":190,\"minTemp\":180,\"time\":2},{\"maxPress\":100,\"minPress\":80,\"maxTemp\":190,\"minTemp\":180,\"time\":480},{\"maxPress\":100,\"minPress\":80,\"maxTemp\":190,\"minTemp\":180,\"time\":120},{\"maxPress\":100,\"minPress\":80,\"maxTemp\":190,\"minTemp\":180,\"time\":120}]},{\"name\":\"Программа 2\",\"stages\":[{\"maxPress\":120,\"minPress\":80,\"maxTemp\":220,\"minTemp\":200,\"time\":10},{\"maxPress\":400,\"minPress\":390,\"maxTemp\":200,\"minTemp\":190,\"time\":10},{\"maxPress\":0,\"minPress\":0,\"maxTemp\":190,\"minTemp\":180,\"time\":2},{\"maxPress\":100,\"minPress\":80,\"maxTemp\":190,\"minTemp\":180,\"time\":480},{\"maxPress\":100,\"minPress\":80,\"maxTemp\":190,\"minTemp\":180,\"time\":120},{\"maxPress\":100,\"minPress\":80,\"maxTemp\":190,\"minTemp\":180,\"time\":150}]}]";

String facttoryParams = "[{\"protection\": 12,\"beeper\": false, \"sensor\": 60, \"ssid\": \"name\", \"password\": \"password\"}]";


bool isADCConnected, wasStartedStockUp, screenBeginFlag, webStartFlag, saveSetsFlag;

String dnsName = "maslobot";
bool isConnectedToRouter;

String getSensorReadings(){
  pork["timeLeftHour"] = myTime.leftHour; 
  pork["timeLeftMins"] = myTime.leftMins;
  pork["timeLeftSec"] = myTime.leftSec;
  pork["maxPress"] = sensor.maxPress;
  pork["minPress"] = sensor.minPress;
  pork["maxTemp"] = sensor.maxTemp;
  pork["minTemp"] = sensor.minTemp;
  pork["pastHours"] = myTime.past.hours;
  pork["pastMins"] = myTime.past.mins;
  pork["pastSec"] = myTime.past.sec;
  pork["pressure"] = sensor.pressure;
  pork["temperature"] = sensor.temp;
  
  String jsonWS;
  serializeJson(pork, jsonWS);
  return jsonWS;
}

int tableIndex, rowIndex;

int currentStage;

#include "max6675.h"

byte thermoDO = 16;
byte thermoCS = 17;
byte thermoCLK = 18;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

GTimer tempTimer(MS, 500);

GTimer pumpSwitchTmr(MS);  // таймер для задержки переключения помп

MCPBlinker blinker(BLINK_LED);

