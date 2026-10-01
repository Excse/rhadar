#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "components/sensor.h"
#include "connection.h"
#include "message.h"
#include "device.h"
#include "origin.h"

void require(bool condition, const char* message) {
    if (condition) return;
    std::cerr << message << '\n';
    std::abort();
}

void constructs_entire_message() {
    auto device = rhadar::DeviceBuilder("pws_abc123")
        .name("Plant watering system")
        .manufacturer("DIY")
        .model("ESP32")
        .add_connection("mac", "02:5b:26:a8:dc:12")
        .build();
    require(device.has_value(), "device construction failed");

    auto origin = rhadar::OriginBuilder("plant-watering-system")
        .sw_version("1.0.0")
        .build();
    require(origin.has_value(), "origin construction failed");

    auto moisture = rhadar::SensorBuilder("pws_abc123_moisture")
        .state_topic("pws_abc123/moisture/state")
        .name("Soil moisture")
        .device_class(rhadar::SensorDeviceClass::Moisture)
        .state_class(rhadar::SensorStateClass::Measurement)
        .unit_of_measurement("%")
        .availability_topic("pws_abc123/status")
        .force_update(false)
        .suggested_display_precision(0)
        .build();
    require(moisture.has_value(), "moisture sensor construction failed");

    auto temperature = rhadar::SensorBuilder("pws_abc123_temperature")
        .state_topic("pws_abc123/temperature/state")
        .name("Temperature")
        .device_class(rhadar::SensorDeviceClass::Temperature)
        .state_class(rhadar::SensorStateClass::Measurement)
        .unit_of_measurement("C")
        .build();
    require(temperature.has_value(), "temperature sensor construction failed");

    std::vector<rhadar::Component> components{
        {"moisture", *moisture},
        {"temperature", *temperature},
    };

    auto message = rhadar::MessageBuilder(*device, *origin, "pws_abc123")
        .node_id("greenhouse")
        .components(std::move(components))
        .qos(1)
        .build();
    require(message.has_value(), "message construction failed");

    require(message->topic() == "homeassistant/device/greenhouse/pws_abc123/config", "unexpected discovery topic");
    require(message->qos() == 1, "unexpected discovery QoS");
    require(message->retain(), "message must be retained");

    const std::string expected_payload =
        "{\"device\":{\"identifiers\":[\"pws_abc123\"],"
        "\"name\":\"Plant watering system\","
        "\"connections\":[[\"mac\",\"02:5b:26:a8:dc:12\"]],"
        "\"manufacturer\":\"DIY\",\"model\":\"ESP32\"},"
        "\"origin\":{\"name\":\"plant-watering-system\","
        "\"sw_version\":\"1.0.0\"},"
        "\"components\":{"
        "\"moisture\":{\"platform\":\"sensor\","
        "\"unique_id\":\"pws_abc123_moisture\","
        "\"state_topic\":\"pws_abc123/moisture/state\","
        "\"name\":\"Soil moisture\",\"device_class\":\"moisture\","
        "\"force_update\":false,\"options\":[],\"suggested_display_precision\":0,"
        "\"state_class\":\"measurement\",\"unit_of_measurement\":\"%\","
        "\"availability_topic\":\"pws_abc123/status\"},"
        "\"temperature\":{\"platform\":\"sensor\","
        "\"unique_id\":\"pws_abc123_temperature\","
        "\"state_topic\":\"pws_abc123/temperature/state\","
        "\"name\":\"Temperature\",\"device_class\":\"temperature\","
        "\"options\":[],\"state_class\":\"measurement\",\"unit_of_measurement\":\"C\"}}}";

    require(message->payload() == expected_payload, "unexpected discovery payload");
}

void rejects_missing_unique_id() {
    auto invalid = rhadar::SensorBuilder("").state_topic("state").build();
    require(!invalid && invalid.error().code == rhadar::ValidationErrorCode::MissingUniqueId,
            "empty unique_id must fail sensor validation");

    rhadar::Sensor absent;
    auto error = rhadar::validate(absent);
    require(error && error->code == rhadar::ValidationErrorCode::MissingUniqueId,
            "default sensor must fail validation without dereferencing an absent id");

    auto device = rhadar::DeviceBuilder("device").build();
    auto origin = rhadar::OriginBuilder("origin").build();
    require(device.has_value() && origin.has_value(), "metadata construction failed");
    auto message = rhadar::MessageBuilder(*device, *origin, "object")
        .add_component("sensor", absent).build();
    require(!message && message.error().code == rhadar::ValidationErrorCode::MissingUniqueId,
            "message builder must reject a missing unique_id before serialization");
}

void serializes_connections_and_durations() {
    auto device = rhadar::DeviceBuilder("device")
        .add_connection("custom\"", "line\nidentifier")
        .add_connection("mac", "aa:bb").build();
    auto origin = rhadar::OriginBuilder("origin").build();
    auto sensor = rhadar::SensorBuilder("id\"\n")
        .state_topic("state").expire_after(std::chrono::seconds{123}).build();
    require(device.has_value() && origin.has_value() && sensor.has_value(),
            "configuration construction failed");
    auto message = rhadar::MessageBuilder(*device, *origin, "object")
        .add_component("sensor", *sensor).build();
    require(message.has_value(), "message construction failed");
    require(message->payload() ==
        R"({"device":{"identifiers":["device"],"connections":[["custom\"","line\nidentifier"],["mac","aa:bb"]]},"origin":{"name":"origin"},"components":{"sensor":{"platform":"sensor","unique_id":"id\"\n","state_topic":"state","expire_after":123,"options":[]}}})",
        "connections, unique_id escaping, durations or absent fields changed");

    auto plain_device = rhadar::DeviceBuilder("device").build();
    require(plain_device.has_value(), "device construction failed");
    auto plain_message = rhadar::MessageBuilder(*plain_device, *origin, "object")
        .add_component("sensor", *sensor).build();
    require(plain_message.has_value() && plain_message->payload().find("connections") == std::string::npos,
            "empty connections must be omitted");
}

int main() {
    constructs_entire_message();
    rejects_missing_unique_id();
    serializes_connections_and_durations();
    return 0;
}
