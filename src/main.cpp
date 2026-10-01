#include <iostream>

#include "rhadar.h"

int main() {
    auto sensor = rhadar::SensorBuilder{"example_temperature"}
        .state_topic("example/temperature/state")
        .device_class(rhadar::SensorDeviceClass::Temperature)
        .build();

    if (!sensor) {
        std::cerr << sensor.error().message << '\n';
        return 1;
    }

    std::cout << sensor->state_topic() << '\n';
    return 0;
}
