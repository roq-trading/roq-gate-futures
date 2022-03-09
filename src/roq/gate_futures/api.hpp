/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

namespace roq {
namespace gate_futures {

struct API final {
  std::string_view get_contracts;
  std::string_view get_order_book;
  // factory
  static API create();
};

}  // namespace gate_futures
}  // namespace roq
