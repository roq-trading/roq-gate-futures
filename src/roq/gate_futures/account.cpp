/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/gate_futures/account.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/clock.hpp"

namespace roq {
namespace gate_futures {

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name) : name{name}, crypto_{config.get_api_key(name), config.get_secret(name)} {
}

std::string Account::create_headers(web::http::Method method, std::string_view const &path, std::string_view const &query, std::string_view const &body) {
  auto now = clock::get_realtime<std::chrono::seconds>();
  return crypto_.create_headers(method, path, query, body, now);
}

std::string Account::create_signature_login(
    std::string_view const &event, std::string_view const &channel, std::string_view const &req_param, std::chrono::seconds timestamp) {
  return crypto_.create_signature_login(event, channel, req_param, timestamp);
}

std::string Account::create_signature(std::string_view const &channel, std::string_view const &event, std::chrono::seconds timestamp) {
  return crypto_.create_signature(channel, event, timestamp);
}

}  // namespace gate_futures
}  // namespace roq
