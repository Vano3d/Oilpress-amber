#include <Arduino.h>

void handleFactoryReset(AsyncWebServerRequest *request)
{
  LittleFS.begin();
  File file = LittleFS.open("/sets.txt", "w");
  file.print(factorySettings);
  file.close();
  request->send(200, "text/plain", "Factory settings applied successfully");
  fsDeserialise();
  serializeJson(doc, jsonString);
  tftMainScreen();
  Serial.println("Factory settings applied successfully");
}


// сохранение параметров отжима
void handleSendFile(AsyncWebServerRequest *request)
{
  const AsyncWebParameter *dataParam = request->getParam("data", true);
  if (request->hasParam("data", true))
  {
    LittleFS.remove("/sets.txt");
    File f = LittleFS.open("/sets.txt", "w");
    f.print(dataParam->value());
    f.close();
    request->send(200, "text/html", "OK");
    saveSets();
    // saveSetsFlag = true;

  }
}

void handleSendSettings(AsyncWebServerRequest *request)
{
  if (request->hasParam("data", true))
  {
    buzzFlag = 0;
    const AsyncWebParameter *p = request->getParam("data", true);
    LittleFS.remove("/params.txt");
    File s = LittleFS.open("/params.txt", "w");
    s.print(p->value());
    s.close();
    File set = LittleFS.open("/params.txt", "r");
    DeserializationError error = deserializeJson(sok, set);
    set.close();

    // Обновляем файл для отображения в web
    serializeJson(sok, jsonParams);
    updParams();

    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      request->send(500, "text/plain", "Failed to deserialize JSON");
      return;
    }

    request->send(200, "text/html", "OK");
    Serial.println("Setting saved successfully");
  }
  else
  {
    request->send(400, "text/plain", "Missing 'data' parameter");
  }
}

void handleStartForm(AsyncWebServerRequest *request)
{
  if (request->hasParam("table") && request->hasParam("row") && !wasStartedFlag)
  {
    int tableIndex = request->getParam("table")->value().toInt();
    int rowIndex = request->getParam("row")->value().toInt();

    chozenSeed = tableIndex;
    mySeed.updateSeed(chozenSeed);
    

    myTime.totalHour = 0;
    myTime.leftMins = 0;

    myTime.end = mySeed.calcEndTime();
    myTime.totalHour = myTime.end / 3600;
    myTime.leftMins = (myTime.end - myTime.totalHour * 3600) / 60;  

    if (doc[chozenSeed]["stages"].is<JsonArray>())
    {
      JsonArray valueArray = doc[chozenSeed]["stages"];
      if (valueArray.size() > rowIndex - 3)
      {
        continueTime = valueArray[rowIndex - 3].as<int>() * 60;

      }
      else
      {
        continueTime = 0;
      }
    }
    else
    {
      Serial.println("Invalid seed structure.");
      request->send(400, "text/plain", "Invalid seed structure.");
      return;
    }
    // поднимаем флаг на запуск процесса
    webStartFlag = true;
    request->send(200, "text/plain", "Command received");
    Serial.print("Handler Serial: ");
    Serial.println(chozenSeed);
    Serial.print("Total hour: ");
    Serial.println(myTime.leftHour);
    Serial.print("Total mins: ");
    Serial.println(myTime.leftMins);
    
  }
  else
  {
    String reason = "Reason: ";
    if (!request->hasParam("table"))
      reason += "'table' parameter missing. ";
    if (!request->hasParam("row"))
      reason += "'row' parameter missing. ";
    if (wasStartedFlag)
      reason += "Process already started. ";

    Serial.println(reason);
    request->send(400, "text/plain", "Invalid request. " + reason);
  }
}

void handleWebStop(AsyncWebServerRequest *request)
{
  if (wasStartedFlag)
  {
    stopProcess();
    endScreen();
    // tftMainScreen();
    stopLed.stop();
  }

  request->send(200, "text/plain", "Web stopped");
  Serial.println("Stopped via web");
}


void notifyClients(String sensorReadings)
{
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    String sensorReadings = getSensorReadings();
    notifyClients(sensorReadings);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
