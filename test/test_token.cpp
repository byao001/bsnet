//
// Created by byao on 12/21/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#include "../src/token.hpp"
#include <gtest/gtest.h>
#include <queue>
#include <random>
using namespace std;
using namespace bsnet;

TEST(TokenTest, token_and_pool) {

uint32_t size = 1024;
TokenPool pool(size);
vector<Token> tokens;
for (int i = 0; i < 1024; ++i) {
  Token tok = pool.alloc_token();
  EXPECT_EQ(static_cast<uint64_t>(tok), i);
  tokens.push_back(tok);
}

for (int i = 0; i < 1024; ++i) {
  pool.free_token(tokens[i]);
}

}