/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/gate_futures/tools/crypto.hpp"

#include <fmt/format.h>

#include "roq/utils/codec/hex.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace tools {

// === CONSTANTS ===

namespace {
auto const PREFIX = "/api/v4"sv;
}

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &key, std::string_view const &secret) : key_{key}, mac_{secret} {
}

std::string Crypto::create_headers(
    web::http::Method method, std::string_view const &path, std::string_view const &query, std::string_view const &body, std::chrono::seconds timestamp) {
  assert(!std::empty(path));
  auto query_2 = [&]() -> std::string_view {
    if (std::empty(query))
      return {};
    assert(query[0] == '?');
    return query.substr(1);
  }();
  hash_.clear();
  hash_.update(body);
  auto tmp_1 = hash_.final(signature_);
  std::string signature_1;
  utils::codec::Hex::encode(signature_1, tmp_1);
  auto tmp = fmt::format("{}\n{}{}\n{}\n{}\n{}"sv, method, PREFIX, path, query_2, signature_1, timestamp.count());
  mac_.clear();
  mac_.update(tmp);
  auto digest = mac_.final(digest_);
  std::string signature_2;
  utils::codec::Hex::encode(signature_2, digest);
  auto result = fmt::format(
      "KEY: {}\r\n"
      "Timestamp: {}\r\n"
      "SIGN: {}\r\n"sv,
      key_,
      timestamp.count(),
      signature_2);
  return result;
}

std::string Crypto::create_signature_login(
    std::string_view const &event, std::string_view const &channel, std::string_view const &req_param, std::chrono::seconds timestamp) {
  auto tmp = fmt::format("{}\n{}\n{}\n{}"sv, event, channel, req_param, timestamp.count());
  mac_.clear();
  mac_.update(tmp);
  auto digest = mac_.final(digest_);
  std::string signature_2;
  utils::codec::Hex::encode(signature_2, digest);
  return signature_2;
}

std::string Crypto::create_signature(std::string_view const &channel, std::string_view const &event, std::chrono::seconds timestamp) {
  auto tmp = fmt::format("channel={}&event={}&time={}"sv, channel, event, timestamp.count());
  mac_.clear();
  mac_.update(tmp);
  auto digest = mac_.final(digest_);
  std::string signature_2;
  utils::codec::Hex::encode(signature_2, digest);
  return signature_2;
}

}  // namespace tools
}  // namespace gate_futures
}  // namespace roq
