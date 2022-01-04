/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

namespace roq {
namespace gate_io_futures {

struct API final {
  std::string_view get_contracts;
  // factory
  static API create();
};

}  // namespace gate_io_futures
}  // namespace roq
