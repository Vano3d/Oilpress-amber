#include <Arduino.h>
#include <ArduinoJson.h>
String SendHTML(){
String ptr;
ptr += R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Маслобот WiFi</title>
  <style>
)rawliteral";

#include <css.h>
// LittleFS.begin();
// if (LittleFS.exists("/styles.css")) {
//   File cssFile = LittleFS.open("/styles.css", "r");
//   ptr += cssFile.readStringUntil('\n');
//   cssFile.close();
// } else {
//   Serial.println("CSS reading error");
// }
ptr += F(R"rawliteral(
  </style>
</head>
<body><div class="page"><div id="header-block"><header class="header"><h1 class="title">Маслобот WiFi</h1><p class="version">Версия 3.0</p><p class="subtitle">Автоматика для маслопресса с возможностью изменять параметры отжима по WiFi</p></header></div><section class="process" id="process_block"><div class="process_left_time"><span id="timeLeftHour"></span><span>:</span><span id="timeLeftMins"></span><span>:</span><span id="timeLeftSec"></span><p class="process_subtitle">Времени осталось</p></div><div class="process_past_time"><span>Времени прошло:&nbsp;</span><span id="pastHours"></span><span>&nbsp;:&nbsp;</span><span id="pastMins"></span><span>&nbsp;:&nbsp;</span><span id="pastSec"></span></div><div class="process_diapazons"><span>Диапазон давлений: </span><span id="maxPress"></span><span> - </span><span id="minPress"></span><span> бар </span></div><div class="process_pressure"><span>Давление в системе: </span><span id="pressure" class="pressure-value"></span><span> бар </span></div></section><div id="middle-block"><div class="button-container"><button class="add-culture-btn" onclick="addCulture()">Добавить культуру</button><button class="check-btn" onclick="checkTables()">Проверить</button><button class="factory-reset-btn">Заводские настройки</button><button class="save-btn" onclick="saveData()">Сохранить!</button></div><div class="links-container"><div><a id="rulesBtn" class="settingslink">Правила</a></div><div><a id="settingsBtn" class="settingslink">Параметры</a></div></div></div><div class="table-container"></div><div id="bottom-block" class="bottom-blk"><button class="check-btn-bottom" onclick="checkTables()">Проверить</button><section class="rules" id="rulessection"><h2 class="collapsible"><span class="toggle-icon">&#9650;</span> Правила установки параметров</h2><div class="rules_list"><p>В таблицах с каждой культурой представлены диапазоны давлений и время перехода к следующему диапазону давлений <strong>в минутах</strong>. Например, программа с тыквой начинает отжим в диапазоне 60..40 бар. К концу 1-й минуты переходит к диапазону 100..80 бар и так далее. </p><p>Для каждой культуры заданы параметры, по которым вы точно получите масло с ожидаемым выходом за время, определённое программой (значение времени в последней строке таблицы). </p><p>Однако в зависимости от разных параметров &ndash; влажность, условия хранение, время года, сменился поставщик культуры, или вам может понадобитсья ускорить или наоборот замедлить процесс отжима масла. Это легко можно сделать изменением параметров в таблице. </p><p>Например, вы не хотите держать давление 100 бар на тыкве целых 8 минут &ndash; исправьте число "8" на меньшее значение, и тогда переход к следующему диапазону произойдёт когда вы захотите. </p><p>Или согласно вашему тех. процессу вы начинаете давить лён со 100 бар &ndash; просто удалите строку с диапазоном 50 бар. </p><p>Хоть в этой программе и есть проверка на некоторые некорректные параметры, настоятельно рекомендуется соблюдать чёткие правила.</p><h3>Правило 0</h3><p><span style="font-weight: bold; color: red;">Нельзя устанавливать значение давления более 600 бар, если у вас датчик давления на 600 бар.</span> Иначе автоматика не сможет зафиксировать давление и маслостанция в какой-то момент просто не остановится. Некоторые модели автоматики могут комплектоваться датчиком на 700 бар. </p><h3>Правило 1</h3><p>Максимальное давление в одной строке всегда больше минимального. Для таких случаев работает проверка, но на всякий случай вы должны это знать. </p><h3>Правило 2</h3><p>Время перехода всегда увеличивается, то есть нельзя во второй строке установить время меньше, чем в 1-й. Тут тоже работает проверка. </p><h3>Правило 3</h3><p>Диапазоны давлений не должны пересекаться или "соприкасаться". Например:</p><div class="sample"><span style="font-weight: bold; color: green;">200</span>-180, 10 мин<br> 250-<span style="font-weight: bold; color: green;">220</span>, 15 мин &ndash; так правильно<br><br><span style="font-weight: bold; color: red;">200</span>-180, 10 мин<br> 250-<span style="font-weight: bold; color: red;">190</span>, 15 мин &ndash; так неправильно</div><h3>Правило 4</h3><p>Количество диапазонов может быть от 5 до 12 включительно. Впрочем, и тут работает проверка. </p></div><h2>Как сохранить параметры</h2><p>После изменения какого-либо параметра, добавления/удаления таблицы или строк нажмите вверху кнопку "Проверить". После успешной проверки отобразится кнопка "Сохранить!", по нажатию на которую контроллер тут же обновится с новыми данными и будет готов к работе. </p><h2>Как восстановить параметры</h2><p>Если вы безнадёжно накосячили, удалили лишнее или что-то работает не так, можно легко восстановить настройки отжима по умолчанию &ndash; нажмите кнопку "Заводские настройки". Загрузятся все культуры и значение отжима, с которыми контроллер поставляется на момент продажи. </p><h2>Инструкция пользователя</h2><p>Находится здесь <a href="https://sunsok.ru/automation/instruction">https://sunsok.ru/automation/instruction</a> &mdash; можно скачать только с WiFi с интернетом. </p><h2>Связь с разработчиком</h2><p>телефон, вотсап: 8-916-325-44-49 Иван<br> телеграм: @Vano3d<br> электропочта: vano3d@gmail.com </p><a href="" onclick="saveJSON()">Скачать файл с настройками</a></section><section class="settings" id="settings"><h2>Настройки</h2><label class="form-switch"><input type="checkbox" id="protectionSwitch"><i></i> Аварийная защита </label><br><p class="timerangetext"> Если маслостанция работает не выключаясь установленное количество секунд, программа отжима останавливается. Предназначенно для непредвиденных случаев, например, если забыли перевести рукоятку на маслостанции в рабочее положение или случился серьёзный прорыв шланга РВД. </p><p class="timerangetext">Время срабатывания в секундах:</p><label><input class="timerange" type="range" id="protectionTime" min="5" max="30" value="10" disabled><span id="protectionTimeValue">10</span></label><br><br><hr class="hr-simple"><label class="form-switch"><input type="checkbox" id="beeperSwitch"><i></i> Звуковой сигнал </label><p>Один сигнал при включении и при старте; 3 сигнала по окончанию программы</p><hr class="hr-simple"><h3>Настройка WiFI</h3><p>Введите параметры вашей домашей WiFI сети, тогда вы сможете обращаться к контроллеру с любого другого устройства, подключённого к этой же сети. Для этого нужно будет набрать в браузере адрес, отображённый на 1-м экране при загрузке контроллера. </p><div id="wifiCredentials"><label class="wifilabels">Имя сети (SSID):</label><br><input maxlength="20" type="text" id="ssid" class="wide-input"><br><label class="wifilabels">Пароль:</label><br><input maxlength="20" type="text" id="password" class="wide-input"><br></div><p>Если контроллеру не удастся подключиться домашней сети, то будет создана точка доступа, адреса доступа отобразятся на 1-м экране загрузки контроллера </p><p>После ввода или изменения параметров WiFi нажмите кнопку "Сохранить параметры" и перезагрузите контроллер</p><p>Если нужно вернуть подключение к контроллеру как к точке доступа, просто сотрите поля с названием сети и паролем, нажмите "Сохранить параметры" и перезагрузите контроллер </p><div class="dangerzone"><hr><p class="pressureDiv"> Датчик давления</p><p>Внимание! Изменяя эту настройку, вы должны быть точно уверены, что у вас установлен соответствующий датчик давления, иначе корректная работа устройства не гарантируется. </p><select id="pressureDiv" class="pressureselect"><option value="60">600</option><option value="70">700</option><option value="80">800</option></select><span class="mpa"> бар</span><hr></div><br><button onclick="saveSets()" class="check-btn ">Сохранить настройки</button><div class="update-container"><h3 class="upd_title">Обновление прошивки</h3><button class="update-button" onclick="document.location='/update'">Перейти к обновлению</button></div></section><br><br></div>
<script>
const data = 

)rawliteral");

ptr += jsonString;
ptr += ";";

ptr += 
R"rawliteral(
    const paramsData = 
)rawliteral";
ptr += jsonParams;
ptr += ";";

#include <js.h>

ptr += 
R"rawliteral(
    </script>
</body>
</html>
)rawliteral";

  return ptr;
}

