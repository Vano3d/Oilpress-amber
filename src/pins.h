// Энкодер
#define S1 35 
#define S2 36 
#define KEY 34 

// индикатор давления
#define CLK 19
#define DIO 23

// индикатор температуры
#define CLK2 4
#define DIO2 27

#define BUZZER 13

#define STARTBUTTON 32 // mcp
#define STOPBUTTON 33 // mcp

// Выходы расширителя портов MCP23017
#define PUMP_LED 0 // реле и светодиод подачи давления
#define HEAT_LED 1 // реле и светодиод тэна
#define PUMP_ZERO 2 // реле и светодиод сброса давления
#define BLINK_LED 3 // светодиод остановки/аварии

// модуль термопары MAX6675
byte thermoDO = 16;
byte thermoCS = 17;
byte thermoCLK = 18;

/* Подключение TFT
SCL 14
SDA(MOSI) 26 
CS 5
DC 25
RST 15 
*/