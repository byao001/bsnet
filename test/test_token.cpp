//
// Created by byao on 12/21/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#include "../src/token.hpp"
#include <gtest/gtest.h>
#include <random>
#include <unordered_set>
#include <vector>
using namespace std;
using namespace bsnet;

TEST(TokenTest, token_alloc) {

  uint32_t size = 1000;
  TokenPool pool(size);
  vector<Token> tokens;
  for (int i = 0; i < 1024; ++i) {
    Token tok = pool.alloc_token();
    EXPECT_EQ(static_cast<uint64_t>(tok), i);
    tokens.push_back(tok);
  }
  EXPECT_THROW(pool.alloc_token(), token_exhuasted);

  for (int i = 0; i < 1024; ++i) {
    pool.free_token(tokens[i]);
  }
}

TEST(TokenTest, random_taken) {
  unordered_set<Token> tokens;
  uint32_t size = 1000;
  TokenPool pool(size);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1);

  while (size--) {
    if (dis(gen)) {
      auto tok = pool.alloc_token();
      EXPECT_TRUE(tokens.find(tok) == tokens.end());
      tokens.insert(tok);
    } else {
      auto tok_it = tokens.begin();
      if (tok_it != tokens.end()) {
        pool.free_token(*tok_it);
        tokens.erase(tok_it);
      }
    }
  }
}