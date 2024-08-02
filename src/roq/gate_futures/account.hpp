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

  Account(Account &&) = default;
  Account(Account const &) = delete;

  std::string create_signature_api_v1(web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body);
  std::string create_signature_api_v2(web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body);

  std::string const name;

 private:
  tools::Crypto crypto_;
};

}  // namespace gate_futures
}  // namespace roq
