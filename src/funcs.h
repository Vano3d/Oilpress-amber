#include <Arduino.h>

void fsDeserialise() {
    File file = LittleFS.open("/sets.txt","r");
    DeserializationError error = deserializeJson(doc, file);
    file.close();
}

void pump_on() {
  digitalWrite(PUMP, 1);
  // при каждом включении помпы запускаем таймер
  pumpOnTmr.setTimeout(safetyTime*1000);
  // barrelOn();
}

void pump_off() {
  // если помпа благополучно выключается, таймер останавливается
  digitalWrite(PUMP, 0);
  pumpOnTmr.stop();
  // barrelOff();
}

void heat_on() {
  digitalWrite(HEAT, 1);
}

void heat_off() {
  digitalWrite(HEAT, 0);
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
  // totalTime = 0;
  endFlag = 0;
  timeBeforeStart = millis() / 1000ul;
  wasStartedFlag = 1;
  isFilled = 0;
  stopLed.stop();
  currentScreen = 4;
  if (beeperFlag) tone(BUZZER, 1500, 150);
  Serial.print("Запущена культура № ");
  Serial.println(chozenSeed);
  processScreen();

}

void stopProcess() {
  pump_off();
  buzzFlag = 1;
  endTimer.setTimeout(100);
  endFlag = 1;
  wasStartedFlag = 0;
  totalTime = 0;
  continueTime = 0;
  Serial.println("Отжим остановлен");
  currentScreen = 6;
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
    vTaskDelay(1500 / portTICK_PERIOD_MS);
  
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




// unsigned long ota_progress_millis = 0;

// void onOTAStart() {
//   // Log when OTA has started
//   Serial.println("OTA update started!");
//   stopProcess();
// }

// void onOTAProgress(size_t current, size_t final) {
//   // Log every 1 second
//   if (millis() - ota_progress_millis > 1000) {
//     ota_progress_millis = millis();
//     Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
//   }
// }

// void onOTAEnd(bool success) {
//   // Log when OTA has finished
//   if (success) {
//     Serial.println("OTA update finished successfully!");
//   } else {
//     Serial.println("There was an error during OTA update!");
//   }
//   // <Add your own code here>
// }
