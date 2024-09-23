/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/gate_futures/config.hpp"

#include "roq/gate_futures/tools/crypto.hpp"

namespace roq {
namespace gate_futures {

struct Account final {
  Account(Config const &, std::string_view const &name);

  Account(Account const &) = delete;

  auto &get_key() const { return crypto_.get_key(); }

  std::string create_headers(web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body);

  std::string create_signature(std::string_view const &channel, std::string_view const &req_param, std::chrono::seconds timestamp);

  std::string const name;

 private:
  tools::Crypto crypto_;
};

}  // namespace gate_futures
}  // namespace roq
