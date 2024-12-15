#include <Arduino.h>

class MySeed {
public:
    MySeed(byte seed) : chozenSeed(seed) {}
    void updateSeed(byte seed) {
        chozenSeed = seed;
    }

    int getCurrentStage(long currentTime) {
        long accumulatedTime = 0;
        size_t stagesCount = length();

        for (int i = 0; i < stagesCount; i++) {
            accumulatedTime += time(i) * 60; // Переводим минуты в секунды
            if (currentTime < accumulatedTime) {
                return i;
            }
        }
        // Если время превышает все этапы, возвращаем последний этап
        return stagesCount - 1;
    }

    int maxPress(byte number) {
        return doc[chozenSeed]["stages"][number]["maxPress"].as<int>();
    }
    int minPress(byte number) { 
        return doc[chozenSeed]["stages"][number]["minPress"].as<int>(); 
    }
    int maxTemp(byte number) {
        return doc[chozenSeed]["stages"][number]["maxTemp"].as<int>();
    }
    int minTemp(byte number) {
        return doc[chozenSeed]["stages"][number]["minTemp"].as<int>();
    }
    int time(byte number) {
        return doc[chozenSeed]["stages"][number]["time"].as<int>();
    }

    size_t length() const {
        return doc[chozenSeed]["stages"].size();
    }
    // size_t lastElement() const {
    //     return doc[chozenSeed]["value"][arrayLen - 1].as<int>() * 60;
    // }
    String name() const {
        return doc[chozenSeed]["name"].as<String>();
    }
    long calcEndTime() {
    long sum = 0;
    int stagesCount = doc[chozenSeed]["stages"].size();
    for (int i = 0; i < stagesCount; i++) {
        sum += doc[chozenSeed]["stages"][i]["time"].as<int>();
    }
    return sum * 60;
}

private:
    byte chozenSeed;
};
MySeed mySeed(0);

class MCPBlinker : public VirtBlinker {
   public:
    MCPBlinker(uint8_t pin) {
        _pin = pin;
    }
    
    bool tick() {
        if (VirtBlinker::tick()) {
            mcp.digitalWrite(_pin, state());
            return true;
        }
        return false;
    }
    
    void stopBlink() {
        stop();  // вызываем метод родительского класса для остановки мигания
        mcp.digitalWrite(_pin, LOW);  // выключаем светодиод
    }
    
   private:
    uint8_t _pin;
};

