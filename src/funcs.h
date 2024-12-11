#include <Arduino.h>

void fsDeserialise() {
    File file = LittleFS.open("/sets.txt","r");
    DeserializationError error = deserializeJson(doc, file);
    file.close();
}

void pump_on() {
  // digitalWrite(PUMP, 1);
  pcf8574.digitalWrite(P0, 0);
  // при каждом включении помпы запускаем таймер
  pumpOnTmr.setTimeout(safetyTime*1000);
  // barrelOn();
}

void pump_off() {
  // если помпа благополучно выключается, таймер останавливается
  // digitalWrite(PUMP, 0);
  pcf8574.digitalWrite(P0, 1);
  pumpOnTmr.stop();

}

void heat_on() {
  // digitalWrite(HEAT, 1);
  pcf8574.digitalWrite(P1, 0);
}

void heat_off() {
  // digitalWrite(HEAT, 0);
  pcf8574.digitalWrite(P1, 1);
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
  sensor.isFilled = 0;
  stopLed.stop();
  currentScreen = PROCESS;
  if (beeperFlag) tone(BUZZER, 1500, 150);
  Serial.print("Запущена культура № ");
  Serial.println(chozenSeed);
  processScreen();
}

void stopProcess() {
  pump_off();
  heat_off();
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