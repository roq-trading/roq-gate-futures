/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/core/utility.h"

#include "roq/core/json/parser.h"

#include "roq/core/charconv.h"
#include "roq/core/charconv/datetime.h"

namespace roq {
namespace gateio_futures {
namespace json {

template <typename T>
inline void update(T &result, const core::json::value_t &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::seconds &result, const core::json::value_t &value) {
  return std::visit(
      overloaded{
          [&](const core::json::null_t &) { result = std::chrono::seconds{}; },
          [](bool) { throw std::bad_cast(); },
          [&](int64_t value) { result = std::chrono::seconds{value}; },
          [&](double value) { result = std::chrono::seconds{static_cast<int64_t>(value)}; },
          [&](const std::string_view &value) {
            result =
                core::charconv::datetime_from_string<std::remove_reference<decltype(result)>::type>(
                    value);
          },
          [](const core::json::object_t &) { throw std::bad_cast(); },
          [](const core::json::array_t &) { throw std::bad_cast(); },
      },
      value);
}

template <>
inline void update(std::chrono::milliseconds &result, const core::json::value_t &value) {
  return std::visit(
      overloaded{
          [&](const core::json::null_t &) { result = std::chrono::milliseconds{}; },
          [](bool) { throw std::bad_cast(); },
          [&](int64_t value) { result = std::chrono::milliseconds{value}; },
          [&](double value) {
            result = std::chrono::milliseconds{static_cast<int64_t>(value * 1000000.0)};
          },
          [&](const std::string_view &value) {
            result =
                core::charconv::datetime_from_string<std::remove_reference<decltype(result)>::type>(
                    value);
          },
          [](const core::json::object_t &) { throw std::bad_cast(); },
          [](const core::json::array_t &) { throw std::bad_cast(); },
      },
      value);
}

template <>
inline void update(std::chrono::microseconds &result, const core::json::value_t &value) {
  return std::visit(
      overloaded{
          [&](const core::json::null_t &) { result = std::chrono::microseconds{}; },
          [](bool) { throw std::bad_cast(); },
          [&](int64_t value) { result = std::chrono::microseconds{value}; },
          [&](double value) {
            result = std::chrono::microseconds{static_cast<int64_t>(value * 1000000.0)};
          },
          [&](const std::string_view &value) {
            auto tmp = core::from_chars<double>(value);
            result = std::chrono::microseconds{static_cast<int64_t>(tmp * 1000000.0)};
          },
          [](const core::json::object_t &) { throw std::bad_cast(); },
          [](const core::json::array_t &) { throw std::bad_cast(); },
      },
      value);
}

template <>
inline void update(std::chrono::nanoseconds &result, const core::json::value_t &value) {
  return std::visit(
      overloaded{
          [&](const core::json::null_t &) { result = std::chrono::nanoseconds{}; },
          [](bool) { throw std::bad_cast(); },
          [&](int64_t value) { result = std::chrono::nanoseconds{value}; },
          [&](double value) {
            result = std::chrono::nanoseconds{static_cast<int64_t>(value * 1.0e9)};
          },
          [&](const std::string_view &value) {
            auto tmp = core::from_chars<double>(value);
            result = std::chrono::nanoseconds{static_cast<int64_t>(tmp * 1.0e9)};
          },
          [](const core::json::object_t &) { throw std::bad_cast(); },
          [](const core::json::array_t &) { throw std::bad_cast(); },
      },
      value);
}

}  // namespace json
}  // namespace gateio_futures
}  // namespace roq
