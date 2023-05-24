#include <string>

#include "KVStore.h"
#include "gtest/gtest.h"

KVStore storage = KVStore("./data");

TEST(StorageTest, single_test) { 
  // const std::string empty_string = "";
  EXPECT_EQ("", storage.get(1));
  storage.put(1, "ysm_data");
  EXPECT_EQ("ysm_data", storage.get(1));
  EXPECT_EQ(true, storage.del(1));
  EXPECT_EQ("", storage.get(1));
  EXPECT_EQ(false, storage.del(1));
}

TEST(StorageTest, small_test) {
  const uint64_t small = 1024;
  // put
  for(uint64_t i = 1; i <= small; i++){
    storage.put(i, std::string(i, 's'));
    EXPECT_EQ(std::string(i, 's'), storage.get(i));
  }
  // get
  for(uint64_t i = 1; i <= small; i++) {
    EXPECT_EQ(std::string(i, 's'), storage.get(i));
  }
  //delete
  for(uint64_t i = 1; i <= small; i++) {
    EXPECT_EQ(true, storage.del(i));
  }
  // get
  for(uint64_t i = 1; i <= small; i++) {
    EXPECT_EQ("", storage.get(i));
  }
}

TEST(StorageTest, large_test) {
  const uint64_t large = 1024 * 64;
  // put
  for(uint64_t i = 1; i <= large; i++){
    storage.put(i, std::string(i, 's'));
    EXPECT_EQ(std::string(i, 's'), storage.get(i));
  }
  // get
  for(uint64_t i = 1; i <= large; i++) {
    EXPECT_EQ(std::string(i, 's'), storage.get(i));
  }
  //delete
  for(uint64_t i = 1; i <= large; i++) {
    EXPECT_EQ(true, storage.del(i));
  }
  // get
  for(uint64_t i = 1; i <= large; i++) {
    EXPECT_EQ("", storage.get(i));
  }
}

int main(int argc, char **argv) {
  printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();   
}


