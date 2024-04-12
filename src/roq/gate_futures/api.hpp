/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/gate_futures/settings.hpp"

namespace roq {
namespace gate_futures {

struct API final {
  std::string_view spot_currencies;
  std::string_view futures_contracts;
  std::string_view futures_order_book;

  // factory
  static API create(Settings const &);
};

}  // namespace gate_futures
}  // namespace roq
