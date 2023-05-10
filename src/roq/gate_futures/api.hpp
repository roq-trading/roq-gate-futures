/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/gate_futures/settings.hpp"

namespace roq {
namespace gate_futures {

struct API final {
  std::string_view get_contracts;
  std::string_view get_order_book;
  // factory
  static API create(Settings const &);
};

}  // namespace gate_futures
}  // namespace roq
