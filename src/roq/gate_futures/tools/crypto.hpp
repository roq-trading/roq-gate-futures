/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <array>
#include <chrono>
#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/utils/mac/hmac.hpp"

namespace roq {
namespace gate_futures {
namespace tools {

struct Crypto final {
  Crypto(std::string_view const &key, std::string_view const &secret);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  std::string create_headers(
      web::http::Method, std::string_view const &path, std::string_view const &query, std::string_view const &body, std::chrono::seconds now);

 private:
  using Hash = utils::hash::SHA512;
  using Signature = std::array<std::byte, Hash::DIGEST_LENGTH>;
  using MAC = utils::mac::HMAC<utils::hash::SHA512>;
  using Digest = std::array<std::byte, MAC::DIGEST_LENGTH>;

  std::string const key_;
  Hash hash_;
  Signature signature_;
  MAC mac_;
  Digest digest_;
};

}  // namespace tools
}  // namespace gate_futures
}  // namespace roq
