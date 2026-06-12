/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/gate_futures/gateway/settings.hpp"

namespace roq {
namespace gate_futures {
namespace gateway {

struct API final {
  std::string_view spot_currencies;
  std::string_view futures_contracts;
  std::string_view futures_order_book;
  std::string_view futures_candlesticks;

  std::string_view accounts;
  std::string_view positions;
  std::string_view orders;
  std::string_view trades;

  // factory
  static API create(Settings const &);

  enum class Key {
    BTC,
    USDT,
  };

  static Key parse_api(Settings const &);
};

}  // namespace gateway
}  // namespace gate_futures
}  // namespace roq
