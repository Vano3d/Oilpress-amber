#include <Arduino.h>

void fsDeserialise() {
    File file = LittleFS.open("/sets.txt","r");
    DeserializationError error = deserializeJson(doc, file);
    file.close();
}

void pump_on() {
  // digitalWrite(PUMP, 1);
  mcp.digitalWrite(PUMP_LED, HIGH);
  // при каждом включении помпы запускаем таймер
  pumpOnTmr.setTimeout(safetyTime*1000);
  // barrelOn();
}

void pump_off() {
  mcp.digitalWrite(PUMP_LED, LOW);
  pumpOnTmr.stop();

}

void heat_on() {
  mcp.digitalWrite(HEAT_LED, HIGH);
}

void heat_off() {
  mcp.digitalWrite(HEAT_LED, LOW);
}

void pumpZero_on() {
  mcp.digitalWrite(PUMP_ZERO, HIGH);
}
void pumpZero_off() {
  mcp.digitalWrite(PUMP_ZERO, LOW);
}

void buzzer() {
  if (buzzFlag) {
    tone(BUZZER, 1000, 650);
    buzzTimer1.setTimeout(1200);
    buzzFlag = 0;
  }

  if (buzzTimer1.isReady()) {
    tone(BUZZER, 1000, 650);
    buzzTimer2.setTimeout(1200);
  }

   if (buzzTimer2.isReady()) {
    tone(BUZZER, 1000, 650);
  }
}


void startProcess() {
  // currentTime = 0;
  endFlag = 0;
  myTime.beforeStart = millis() / 1000ul;
  wasStartedFlag = 1;

  // stopLed.stop();
  blinker.stopBlink();
  currentScreen = PROCESS;
  if (beeperFlag) tone(BUZZER, 1500, 150);
  Serial.print("Запущена культура № ");
  Serial.println(chozenSeed);
  processScreen();
}

void stopProcess() {
  pump_off();
  heat_off();
  sensor.isFilled = 0;
  sensor.isWarmed = 0;
  buzzFlag = 1;
  endTimer.setTimeout(100);
  endFlag = 1;
  wasStartedFlag = 0;
  myTime.current = 0;
  continueTime = 0;
  Serial.println("Отжим остановлен");
  currentScreen = END;
}

void checkADCConnection() {
    // Проверяем, подключен ли датчик
    if (!ADS.begin()) {
      Serial.println("ADS1115 sensor probably not connected :-(");
      isADCConnected = false;
    } else {
      Serial.println("ADS1115 sensor connected!");
      isADCConnected = true;
    }

    // Задержка перед следующей проверкой
    delay(1500);
  
}


void saveSets() {
  
    File file = LittleFS.open("/sets.txt", "r");
    if (!file)
    {
      Serial.println("Failed to open file for reading");
      return;
    }
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    // Обновляем файл для отображения в web
    serializeJson(doc, jsonString);
    Serial.println("Params saved successfully");
    //

    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    
    if (!wasStartedFlag) tftMainScreen();
}

void resetWiFi() {
    sok[0]["ssid"] = "";
    sok[0]["password"] = "";
    String output;
    serializeJson(sok, output);

  if (LittleFS.exists("/params.txt")) {
    File ft = LittleFS.open("/params.txt", "w");
    DeserializationError error = deserializeJson(sok, ft);
    ft.print(output);
    ft.close();
  } else {
        Serial.println("Ошибка открытия файла для записи");
        return;
    }
  }

long calcEndTime(int seed) {
    long sum = 0;
    int stagesCount = doc[seed]["stages"].size();
    for (int i = 0; i < stagesCount; i++) {
        sum += doc[seed]["stages"][i]["time"].as<int>();
    }
    return sum * 60;
}

void updParams() {
  safetyTime = sok[0]["protection"].as <int> ();
  beeperFlag = sok[0]["beeper"].as <bool> ();
  sensorPressure = sok[0]["sensor"].as <int> ();
}

void encRotate(bool isLeft) { 
  if (!wasStartedFlag) {
    switch (currentScreen) {
      case MAIN: // экран с культурами
        chozenSeed = constrain(chozenSeed + (isLeft ? -1 : 1), 0, doc.size() - 1);
        
        if (doc.size() > PROGS_ON_SCREEN && (chozenSeed + 1) % PROGS_ON_SCREEN == 0) {
          cultureScreenNuber = constrain(cultureScreenNuber - 1, 1, cultureScreens);
          firstCulture = constrain(firstCulture - PROGS_ON_SCREEN, 0, (cultureScreens - 1) * PROGS_ON_SCREEN);
          mainScreenUpdate();
        }
        if (isLeft) {
          cursorBack();
        } else {
          cursorForward();
        }
        break;
        
      case DIAPAZONS: // экран с диапазонами
        if (isLeft ? diapScreenNumber > 1 : diapScreenNumber < diapScreens) {
          if (isLeft) {
            diapScreenNumber--;
          } else {
            diapScreenNumber++;
          }
          firstDiap = (diapScreenNumber - 1) * 2;
          diapazonsForParams();
        }
        break;
        
      case END:
      case WIFIINFO:
        currentScreen = MAIN;
        tftMainScreen();
        break;
        
      default:
        break;
    }
  }
}

void encClick() {
      switch (currentScreen) {
    case MAIN: // если на главном экране, переходим на диапазоны
      currentScreen = DIAPAZONS;
      diapazonScreen();
      break;
    case DIAPAZONS:
    case WIFIINFO: // если экран с диапазоном или WiFi, переходим на главный
      currentScreen = MAIN;
      tftMainScreen();
      break;

    default:
      break;
    }
}

void blinkHundredTimes() {
    blinker.blink(100, 200, 600);  // 100 раз, 200мс вкл, 200мс выкл
}

const unsigned long DEBOUNCE_TIME = 50; // Время дебаунса в миллисекундах

// Переменные для хранения состояния кнопок и времени последнего изменения
bool startButtonPressed = false;
bool stopButtonPressed = false;

unsigned long lastStartButtonTime = 0;
unsigned long lastStopButtonTime = 0;


// Функция обработки нажатия кнопки Старт
void onStartButtonPressed() {
  Serial.println("Кнопка 'Старт' нажата");
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
    // stopLed.stop(); // предположительно остановка светодиода
    blinker.stopBlink(); // останавливаем мигание
    currentScreen = PROCESS; // меняем экран на PROCESS
    startProcess(); // запускаем процесс
    break;
  }
}

// Функция обработки нажатия кнопки Стоп
void onStopButtonPressed() {
  Serial.println("Кнопка 'Стоп' нажата");
  stopProcess();
  endScreen();
}

void checkButtons() {
  static bool lastStartButtonState = HIGH;
  static bool lastStopButtonState = HIGH;

  // Проверка кнопки "Старт"
  bool currentStartButtonState = mcp.digitalRead(START_BUTTON);
  if (currentStartButtonState != lastStartButtonState) {
    unsigned long currentTime = millis();
    if ((currentTime - lastStartButtonTime) > DEBOUNCE_TIME) {
      lastStartButtonTime = currentTime;
      lastStartButtonState = currentStartButtonState;
      if (currentStartButtonState == LOW) { // Кнопка нажата
        startButtonPressed = true;
        Serial.println("Кнопка 'Старт' нажата");
      }
    }
  }

  // Проверка кнопки "Стоп"
  bool currentStopButtonState = mcp.digitalRead(STOP_BUTTON);
  if (currentStopButtonState != lastStopButtonState) {
    unsigned long currentTime = millis();
    if ((currentTime - lastStopButtonTime) > DEBOUNCE_TIME) {
      lastStopButtonTime = currentTime;
      lastStopButtonState = currentStopButtonState;
      if (currentStopButtonState == LOW) { // Кнопка нажата
        stopButtonPressed = true;
        Serial.println("Кнопка 'Стоп' нажата");
      }
    }
  }
}
