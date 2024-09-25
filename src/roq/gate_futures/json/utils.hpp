/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/utils/patterns.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/core/json/parser.hpp"

namespace roq {
namespace gate_futures {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::seconds &result, core::json::Value const &value) {
  return std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = std::chrono::seconds{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = std::chrono::seconds{value}; },
          [&](double value) { result = std::chrono::seconds{static_cast<int64_t>(value)}; },
          [&](std::string_view const &value) {
            result = utils::charconv::from_chars<std::remove_reference<decltype(result)>::type>(value, utils::charconv::Format::DATETIME);
          },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  return std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = std::chrono::milliseconds{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = std::chrono::milliseconds{value}; },
          [&](double value) { result = std::chrono::milliseconds{static_cast<int64_t>(value * 1.0e3)}; },
          [&](std::string_view const &value) {
            result = utils::charconv::from_chars<std::remove_reference<decltype(result)>::type>(value, utils::charconv::Format::DATETIME);
          },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

template <>
inline void update(std::chrono::microseconds &result, core::json::Value const &value) {
  return std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = std::chrono::microseconds{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = std::chrono::microseconds{value}; },
          [&](double value) { result = std::chrono::microseconds{static_cast<int64_t>(value * 1.0e6)}; },
          [&](std::string_view const &value) {
            auto tmp = utils::charconv::from_chars<double>(value);
            result = std::chrono::microseconds{static_cast<int64_t>(tmp * 1.0e6)};
          },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

template <>
inline void update(std::chrono::nanoseconds &result, core::json::Value const &value) {
  return std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = std::chrono::nanoseconds{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = std::chrono::nanoseconds{value}; },
          [&](double value) { result = std::chrono::nanoseconds{static_cast<int64_t>(value * 1.0e9)}; },
          [&](std::string_view const &value) {
            auto tmp = utils::charconv::from_chars<double>(value);
            result = std::chrono::nanoseconds{static_cast<int64_t>(tmp * 1.0e9)};
          },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
