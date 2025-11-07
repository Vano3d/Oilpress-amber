#include <Arduino.h>

void fsDeserialise() {
    File file = LittleFS.open("/sets.txt","r");
    DeserializationError error = deserializeJson(doc, file);
    file.close();
}

void pump_on() {
  mcp.digitalWrite(PUMP_LED, HIGH);
  // при каждом включении помпы запускаем таймер
  pumpOnTmr.setTimeout(safetyTime*1000);

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

// функция пищания 3 раза при поднятии флага
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

  // Сбрасываем логическую прогрессию ступеней при старте процесса
  logicalStage = 0;
  lastLogicalStage = -1;
  pressureReached = false;
  tempReached = false;
  stageTimerStartMs = 0;

  blinker.stopBlink();
  currentScreen = PROCESS;

  if (beeperFlag) tone(BUZZER, 1500, 150);
  Serial.print("Запущена культура № ");
  Serial.println(chozenSeed);
  processScreen();
  // Включаем таймер отсчёта времени работы помпы, чтобы сработал аварийный режим
  if (sensor.isFilled) pumpOnTmr.setTimeout(safetyTime*1000);
}

void stopProcess() {
  pump_off();
  heat_off();
  sensor.isFilled = 0;
  sensor.isWarmed = 0;
  preHeatStage = 0;
  buzzFlag = 1; // будет писчать 3 раза
  // endTimer.setTimeout(100);
  endFlag = 1;
  wasStartedFlag = 0;
  myTime.current = 0;
  // Serial.println("Отжим остановлен");
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
    
    if (!wasStartedFlag && !preHeatStage) tftMainScreen();
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
  safetyTime = sok[0]["protection"].as<int>();
  beeperFlag = sok[0]["beeper"].as<bool>();
  sensorPressure = sok[0]["sensor"].as<int>();
  // заменено: deprecated containsKey -> проверка через is<int>()
  if (sok[0]["stopPressure"].is<int>()) {
    stopPressure = sok[0]["stopPressure"].as<int>();
  }
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

void startHeating() {
  if (sensor.temp < sensor.maxTemp) {
    preHeatStage = true;
    preHeatScreen();
    if (beeperFlag) tone(BUZZER, 1500, 150);
    heat_on();
    Serial.println("Включение нагрева");
  } else if (sensor.temp >= sensor.maxTemp) startProcess();
}



void checkPreHeat() {
    if (sensor.temp >= sensor.maxTemp && preHeatStage) {
      preHeatStage = false;
      heat_off();
      Serial.println("Нагрев завершен");
      startProcess();
    }
}

// Универсальная функция контроля для текущей логической ступени
void processStageControl() {
  if (!wasStartedFlag) return; // управляем только в процессе
  int totalStages = mySeed.length();
  if (logicalStage < 0) logicalStage = 0;
  if (logicalStage >= totalStages) return;

  // при входе в новую ступень — сбрасываем внутренние флаги
  if (logicalStage != lastLogicalStage) {
    lastLogicalStage = logicalStage;
    pressureReached = false;
    tempReached = false;
    stageTimerStartMs = 0;
    Serial.print("Entered logical stage: ");
    Serial.println(logicalStage);
  }

  // Получаем параметры текущей логической ступени
  int maxP = mySeed.maxPress(logicalStage);
  int minP = mySeed.minPress(logicalStage);
  int maxT = mySeed.maxTemp(logicalStage);
  int minT = mySeed.minTemp(logicalStage);
  int durationSec = mySeed.time(logicalStage) * 60; // время в секундах

  // ----- Управление давлением -----
  if (maxP == 0 && minP == 0) {
    // режим сброса давления: выключаем основной насос, включаем pumpZero при превышении stopPressure
    pump_off();
    sensor.isFilled = 0;
    if (!pumpSwitchTmr.isEnabled()) pumpSwitchTmr.setTimeout(1000);
    if (pumpSwitchTmr.isReady()) {
      if (sensor.pressure > stopPressure) {
        pumpZero_on();
      } else {
        pumpZero_off();
      }
    }
    // считаем, что давление "достигнуто" когда фактическое давление упало до порога stopPressure или ниже
    if (sensor.pressure <= stopPressure) {
      pressureReached = true;
    }
  } else {
    // поднятие до maxP: пока не достигли maxP -> включаем основной насос
    if (!pressureReached) {
      if (sensor.pressure < maxP) {
        pump_on();
      } else {
        pump_off();
        pressureReached = true;
      }
    } else {
      // когда уже в режиме удержания — поддерживаем между maxP и minP
      if (sensor.pressure >= maxP && sensor.isFilled) {
        pump_off();
        sensor.isFilled = 0;
      } else if (sensor.pressure <= minP && !sensor.isFilled) {
        if (minP != 0) {
          pump_on();
          sensor.isFilled = 1;
        }
      }
    }
    // в обычном режиме вспомогательная помпа должна быть выключена
    mcp.digitalWrite(PUMP_ZERO, LOW);
    pumpSwitchTmr.stop();
  }

  // ----- Управление температурой -----
  // Пока не достигли требуемой температуры — нагреваем до maxT
  if (!tempReached) {
    if (sensor.temp < maxT) {
      heat_on();
    } else {
      heat_off();
      tempReached = true;
    }
  } else {
    // Поддерживаем температуру в пределах minT..maxT
    if (sensor.temp >= maxT && sensor.isWarmed) {
      heat_off();
      sensor.isWarmed = 0;
    } else if (sensor.temp <= minT && !sensor.isWarmed) {
      heat_on();
      sensor.isWarmed = 1;
    }
  }

  // ----- Логика старта таймера и перехода ступеней -----
  if (durationSec > 0) {
    // стандартная ступень: запуск таймера только после достижения давления и температуры
    if (pressureReached && tempReached) {
      if (stageTimerStartMs == 0) {
        stageTimerStartMs = millis();
        Serial.print("Stage timer started for stage ");
        Serial.println(logicalStage);
      } else {
        if ((millis() - stageTimerStartMs) >= (unsigned long)durationSec * 1000UL) {
          // переход на следующую логическую ступень
          logicalStage++;
          if (logicalStage >= totalStages) {
            // завершение процесса
            stopProcess();
            blinkHundredTimes();
            endScreen();
          } else {
            // при переходе — сброс флагов (будет выполнено на следующем вызове)
            Serial.print("Moving to next logical stage: ");
            Serial.println(logicalStage);
          }
        }
      }
    }
  } else {
    // duration == 0 -> переходной/ожидательный режим (обычно охлождение)
    // поведение: ждать понижения температуры до значения nextStage.maxTemp и затем перейти на следующую ступень
    if (logicalStage + 1 < totalStages) {
      int nextMaxT = mySeed.maxTemp(logicalStage + 1);
      if (sensor.temp <= nextMaxT) {
        // переходим на следующую логическую ступень; далее обычная логика поднимет/понизит давление в соответствии с новым этапом
        logicalStage++;
        Serial.print("Auto-advance to next stage after cooling: ");
        Serial.println(logicalStage);
      }
    } else {
      // если следующей ступени нет — считаем задачу завершённой
      logicalStage++;
    }
  }
}

// Сохраняем старую функцию для совместимости (точка входа для preheat, если нужна)
void pressureControlPreHeatMode() {
  // В pre-heat режиме можно просто вызывать общую функцию, она учитывает wasStartedFlag
  processStageControl();
}