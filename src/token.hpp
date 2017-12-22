//
// Created by byao on 12/21/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#pragma once

#include <cstdint>
#include <string>
#include "utility.hpp"

namespace bsnet {

DECL_ERR(token_exhuasted);

typedef std::uint32_t Int;

class Token {
  std::uint64_t _tok;
  explicit Token(std::uint64_t tok);
  explicit operator std::uint64_t() const;
  friend class TokenPool;
};

class TokenPool {
public:
  explicit TokenPool(std::uint32_t size);
  Token alloc_token();
  void free_token(Token tok);

private:
  std::uint32_t _size;
  std::uint32_t _idx;
  Int *_mem;
};

}

namespace std {

template <> struct hash<bsnet::Token> {
  typedef bsnet::Token argument_type;
  typedef std::uint64_t result_type;

  result_type operator()(const argument_type &tok) const noexcept {
    return static_cast<std::uint64_t>(tok);
  }
};
}