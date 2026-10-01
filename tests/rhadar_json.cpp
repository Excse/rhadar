#include <gtest/gtest.h>

#include "utils/json.h"

#include <chrono>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <utility>

using rhadar::JsonArray;
using rhadar::JsonObject;

template <typename T>
concept ObjectInteger = requires(JsonObject& json, T value) { json.integer("key", value); };
template <typename T>
concept ArrayInteger = requires(JsonArray& json, T value) { json.integer(value); };
template <typename T>
concept RawObject = requires(JsonObject& json, T value) { json.object("key", value); };
template <typename T>
concept LvalueFinish = requires(T& json) { json.finish(); };

static_assert(!ObjectInteger<bool> && !ObjectInteger<std::optional<bool>>);
static_assert(!ObjectInteger<double> && !ObjectInteger<std::optional<double>>);
static_assert(!ObjectInteger<std::chrono::duration<double>>);
static_assert(!ObjectInteger<std::optional<std::chrono::duration<double>>>);
static_assert(!ObjectInteger<std::chrono::duration<bool>>);
static_assert(ObjectInteger<std::uint64_t> && ObjectInteger<std::optional<std::uint64_t>>);
static_assert(ObjectInteger<std::chrono::milliseconds>);
static_assert(!ArrayInteger<bool> && !ArrayInteger<double>);
static_assert(!RawObject<std::string> && !RawObject<const char*>);
static_assert(!LvalueFinish<JsonObject>);

namespace rhadar {
enum class TestField { Value };
template <> struct enum_traits<TestField> {
    static constexpr std::string_view to_string(TestField, EnumStringFormat format) {
        return format == EnumStringFormat::Full ? "value" : "v";
    }
};
}

void expect(const std::string& actual, const std::string& expected) {
    EXPECT_EQ(actual, expected);
}

TEST(Json, OptionalFields) {
    JsonObject json;
    json.string("absent", std::optional<std::string>{});
    json.boolean("absent", std::optional<bool>{});
    json.integer("absent", std::optional<int>{});
    json.string("empty", std::optional<std::string>{""});
    json.boolean("false", std::optional<bool>{false});
    json.integer("zero", std::optional<int>{0});
    json.string("enum", std::optional{rhadar::TestField::Value});
    expect(std::move(json).finish(), R"({"empty":"","false":false,"zero":0,"enum":"value"})");
}

TEST(Json, NestedValues) {
    JsonObject json(rhadar::EnumStringFormat::Abbreviated);
    json.object("empty", [](JsonObject&) {});
    json.object("child", [](JsonObject& child) {
        child.array("items", [](JsonArray& items) {
            items.object([](JsonObject& item) { item.integer(rhadar::TestField::Value, 1); });
            items.array([](JsonArray& inner) {
                inner.string("text");
                inner.boolean(false);
                inner.integer(std::numeric_limits<std::uint64_t>::max());
            });
            items.array([](JsonArray&) {});
        });
        child.string("sibling", "ok");
    });
    json.string_array("strings", {"a", "b"});
    json.string_array("empty_array", {});
    json.boolean("last", true);
    expect(std::move(json).finish(), R"({"empty":{},"child":{"items":[{"v":1},["text",false,18446744073709551615],[]],"sibling":"ok"},"strings":["a","b"],"empty_array":[],"last":true})");
}

TEST(Json, Escaping) {
    JsonObject json;
    json.string("\"\\\n", std::string("\"\\\b\f\n\r\t\0\x01\x1f", 10) + "Grüße 🌱");
    json.array("array", [](JsonArray& array) { array.string("\"\n"); });
    expect(std::move(json).finish(), R"({"\"\\\n":"\"\\\b\f\n\r\t\u0000\u0001\u001fGrüße 🌱","array":["\"\n"]})");
}

TEST(Json, IntegersAndDurations) {
    JsonObject json;
    json.integer("min", std::numeric_limits<std::int64_t>::min());
    json.integer("max", std::numeric_limits<std::int64_t>::max());
    json.integer("unsigned", std::optional{std::numeric_limits<std::uint64_t>::max()});
    json.integer("byte", static_cast<unsigned char>(255));
    json.integer("negative", static_cast<signed char>(-128));
    json.integer("seconds", std::optional{std::chrono::seconds{0}});
    json.integer("ms", std::chrono::milliseconds{-123});
    json.integer("missing", std::optional<std::chrono::seconds>{});
    json.integer("unsigned_duration", std::optional{
        std::chrono::duration<std::uint64_t>{std::numeric_limits<std::uint64_t>::max()}});
    expect(std::move(json).finish(), R"({"min":-9223372036854775808,"max":9223372036854775807,"unsigned":18446744073709551615,"byte":255,"negative":-128,"seconds":0,"ms":-123,"unsigned_duration":18446744073709551615})");
}

TEST(Json, EmptyObject) {
    expect(JsonObject{}.finish(), "{}");
}
