//
// Created by byao on 12/21/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#include "token.hpp"
#include <cerrno>
#include <cstring>

using namespace std;

namespace bsnet {

IMPL_ERR(token_exhuasted);

static constexpr int BitsPerInt = sizeof(Int) << 3;

inline constexpr uint64_t FindOffset(size_t pos, int offset) {
  return (pos << 5) + offset;
}

inline constexpr uint32_t NextPowerOf2(uint32_t v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

inline constexpr uint32_t RoundNext(uint32_t value, uint32_t size) {
  return (value + 1) & (size - 1);
}

Token::Token(std::uint64_t tok) : _tok(tok) {}

Token::operator std::uint64_t() const { return _tok; }

bool operator==(Token lhs, Token rhs) { return lhs._tok == rhs._tok; }

TokenPool::TokenPool(uint32_t size)
    : _size(NextPowerOf2(((size - 1) >> 5) + 1)), _idx(0),
      _mem(new Int[_size]()) {}

Token TokenPool::alloc_token() {
  auto idx = _idx;
  while (true) {
    if (_mem[_idx] != std::numeric_limits<Int>::max()) {
      Int v = ~_mem[_idx];
      int count = 0;
      for (; count < BitsPerInt; ++count) {
        int mask = 1 << count;
        if (v & mask) {
          _mem[_idx] |= mask;
          return Token(FindOffset(_idx, count));
        }
      }
    } else {
      if (RoundNext(_idx, _size) == idx) {
        throw token_exhuasted();
      }
      _idx = RoundNext(_idx, _size);
    }
  }
}

void TokenPool::free_token(Token tok) {
  auto v = static_cast<uint64_t>(tok);
  auto off = static_cast<int>(v & (BitsPerInt - 1));
  size_t pos = v >> 5;
  Int mask = (~(1u << off));
  _mem[pos] &= mask;
}
}
