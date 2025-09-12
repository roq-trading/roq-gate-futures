/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/create_order.hpp"

#include "roq/utils/patterns.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/server/oms/order.hpp"

namespace roq {
namespace gate_futures {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::seconds &result, core::json::Value const &value) {
  using result_type = std::remove_cvref_t<decltype(result)>;
  std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = result_type{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = result_type{value}; },
          [&](double value) { result = result_type{static_cast<int64_t>(value)}; },
          [&](std::string_view const &value) { result = utils::charconv::from_chars<result_type>(value, utils::charconv::Format::DATETIME); },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  using result_type = std::remove_cvref_t<decltype(result)>;
  std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = result_type{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = result_type{value}; },
          [&](double value) { result = result_type{static_cast<int64_t>(value * 1.0e3)}; },
          [&](std::string_view const &value) { result = utils::charconv::from_chars<result_type>(value, utils::charconv::Format::DATETIME); },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

template <>
inline void update(std::chrono::microseconds &result, core::json::Value const &value) {
  using result_type = std::remove_cvref_t<decltype(result)>;
  std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = result_type{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = result_type{value}; },
          [&](double value) { result = result_type{static_cast<int64_t>(value * 1.0e6)}; },
          [&](std::string_view const &value) {
            auto tmp = utils::charconv::from_chars<double>(value);
            result = result_type{static_cast<int64_t>(tmp * 1.0e6)};
          },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

template <>
inline void update(std::chrono::nanoseconds &result, core::json::Value const &value) {
  using result_type = std::remove_cvref_t<decltype(result)>;
  std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = result_type{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = result_type{value}; },
          [&](double value) { result = result_type{static_cast<int64_t>(value * 1.0e9)}; },
          [&](std::string_view const &value) {
            auto tmp = utils::charconv::from_chars<double>(value);
            result = result_type{static_cast<int64_t>(tmp * 1.0e9)};
          },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

// helpers

extern std::string_view order_place(
    std::string &buffer, roq::Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id, uint32_t request_id_2);

extern std::string_view order_amend(
    std::string &buffer,
    roq::Event<ModifyOrder> const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    uint32_t request_id_2);

extern std::string_view order_cancel(
    std::string &buffer,
    roq::Event<CancelOrder> const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id,
    uint32_t request_id_2);

extern std::string_view order_cancel_cp(
    std::string &buffer, roq::Event<CancelAllOrders> const &, std::string_view const &request_id, uint32_t request_id_2, std::string_view const &symbol);

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
