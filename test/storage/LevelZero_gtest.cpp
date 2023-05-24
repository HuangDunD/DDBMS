#include <filesystem>
#include <iostream>

#include "gtest/gtest.h"

#include "LevelZero.h"

#include <gtest/gtest.h>

class LEVELZERO_TEST : public ::testing::Test {
 protected:
  void SetUp() override {
    // 在测试之前设置
    if(std::filesystem::exists(dir)) {
      std::filesystem::remove_all(dir);
      std::filesystem::remove(dir);
    }
  }

  void TearDown() override {
    // 在测试之后做清理工作
  }

  const std::string dir = "./level0";
};




TEST_F(LEVELZERO_TEST, empty_test) {
  LevelZero level0(dir, nullptr, nullptr);

  EXPECT_EQ(0, level0.size());
  EXPECT_EQ(std::make_pair(false, std::string("")), level0.search("a"));
}

TEST_F(LEVELZERO_TEST, simple_test) {
  LevelZero level0(dir, nullptr, nullptr);

  const int N = 1024;
  const int OFFSET = 3;
  // start put
  testing::Test::RecordProperty("message", "start puting");
  SkipList memtable;
  for(int offset = 0; offset < OFFSET; offset++) {
    memtable.clear();
    for(int i = 0; i < N; i++) {
      char single_key = 'a' + (char)offset;
      char single_value = ++single_key;
      std::string key(i + 1, single_key);
      std::string value(i + 1, single_value);
      memtable.put(key, value);
    }
    level0.add(memtable, offset);
  }
  testing::Test::RecordProperty("message", "end puting");

  testing::Test::RecordProperty("message", "start search");
  for(int offset = 0; offset < OFFSET; offset++) {
    for(int i = 0; i < N; i++) {
      char single_key = 'a' + (char)offset;
      char single_value = ++single_key;
      std::string key(i + 1, single_key);
      std::string value(i + 1, single_value);
      EXPECT_EQ(std::make_pair(true, value), level0.search(key));
    }
  }
}


TEST_F(LEVELZERO_TEST, large_test) {
  LevelZero level0(dir, nullptr, nullptr);

  const int N = 1024;
  const int OFFSET = 25;
  // start put
  testing::Test::RecordProperty("message", "start puting");
  SkipList memtable;
  for(int offset = 0; offset < OFFSET; offset++) {
    memtable.clear();
    for(int i = 0; i < N; i++) {
      char single_key = 'a' + (char)offset;
      char single_value = ++single_key;
      std::string key(i + 1, single_key);
      std::string value(i + 1, single_value);
      memtable.put(key, value);
    }
    level0.add(memtable, offset);
  }
  testing::Test::RecordProperty("message", "end puting");

  testing::Test::RecordProperty("message", "start search");
  for(int offset = 0; offset < OFFSET; offset++) {
    for(int i = 0; i < N; i++) {
      char single_key = 'a' + (char)offset;
      char single_value = ++single_key;
      std::string key(i + 1, single_key);
      std::string value(i + 1, single_value);
      EXPECT_EQ(std::make_pair(true, value), level0.search(key));
    }
  }
}

int main(int argc, char **argv) {
  printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();   
}