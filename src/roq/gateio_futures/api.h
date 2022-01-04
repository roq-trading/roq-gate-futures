/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

namespace roq {
namespace gateio_futures {

struct API final {
  std::string_view get_contracts;
  // factory
  static API create();
};

}  // namespace gateio_futures
}  // namespace roq
