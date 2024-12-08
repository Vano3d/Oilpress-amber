#include <Arduino.h>

class MySeed {
public:
    MySeed(byte seed) : chozenSeed(seed) {}
    void updateSeed(byte seed) {
        chozenSeed = seed;
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