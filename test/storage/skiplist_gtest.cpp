#include <string>

#include "SkipList.h"
#include "gtest/gtest.h"

SkipList memtable{};
SkipList::Iterator iter(&memtable);

TEST(SKIPLIST_TEST, empty){
    // memtable
    EXPECT_EQ(false, memtable.contains("10"));
    EXPECT_EQ(false, memtable.del("10"));
    // iter
    EXPECT_EQ(false, iter.Valid());
    iter.SeekToFirst();
    EXPECT_EQ(false, iter.Valid());
    iter.SeekToLast();
    EXPECT_EQ(false, iter.Valid());
}

TEST(SKIPLIST_TEST, single_test){
    // before put
    EXPECT_EQ(true, memtable.empty());
    EXPECT_EQ(0, memtable.size());

    memtable.put("Yang", "Shiming");
    // after put
    EXPECT_EQ(std::pair(true, std::string("Shiming")), memtable.get("Yang"));
    EXPECT_EQ(1, memtable.size());
    EXPECT_EQ(false, memtable.empty());
    // iter
    iter.SeekToFirst();
    EXPECT_EQ(true, iter.Valid());
    EXPECT_EQ("Yang", iter.key());
    EXPECT_EQ("Shiming", iter.value());
    iter.SeekToLast();
    EXPECT_EQ(true, iter.Valid());
    EXPECT_EQ("Yang", iter.key());
    EXPECT_EQ("Shiming", iter.value());
    iter.Seek("Yang");
    EXPECT_EQ(true, iter.Valid());
    EXPECT_EQ("Yang", iter.key());
    EXPECT_EQ("Shiming", iter.value());
    iter.Seek("Aang");
    EXPECT_EQ(true, iter.Valid());
    EXPECT_EQ("Yang", iter.key());
    EXPECT_EQ("Shiming", iter.value());
    iter.Next();
    EXPECT_EQ(false, iter.Valid());
    iter.Seek("Yang");
    iter.Prev();
    EXPECT_EQ(false, iter.Valid());
    
    // put
    memtable.put("Yang", "Shi---ming");
    EXPECT_EQ(std::pair(true, std::string("Shi---ming")), memtable.get("Yang"));
    EXPECT_EQ(1, memtable.size());
    EXPECT_EQ(false, memtable.empty());
    // del
    memtable.del("Yang");
    EXPECT_EQ(0, memtable.size());
    EXPECT_EQ(true, memtable.empty());
    EXPECT_EQ(std::pair(false, std::string("")), memtable.get("Yang"));
}

TEST(SKIPLIST_TEST, small_test){
    const int N = 1024;
    
    // put and get
    for(int i = 0; i < N; i++){
        std::string key(i, 'a');
        std::string value(i, 'b');
        memtable.put(key, value);
        EXPECT_EQ(true, memtable.contains(key));
        EXPECT_EQ(std::pair(true, value), memtable.get(key));
    }
    EXPECT_EQ(N, memtable.size());

    // iter
    iter.SeekToFirst();
    for(int i = 0; i < N; i++){
        std::string key(i, 'a');
        std::string value(i, 'b');
        EXPECT_EQ(true, iter.Valid());
        EXPECT_EQ(key, iter.key());
        EXPECT_EQ(value, iter.value());
        iter.Next();
    }
    iter.SeekToLast();
    for(int i = N - 1; i >= 0; i--){
        std::string key(i, 'a');
        std::string value(i, 'b');
        EXPECT_EQ(true, iter.Valid());
        EXPECT_EQ(key, iter.key());
        EXPECT_EQ(value, iter.value());
        iter.Prev();
    }

    // del
    for(int i = 0; i < N; i+=2){
        std::string key(i, 'a');
        EXPECT_EQ(true, memtable.del(key));
        EXPECT_EQ(false, memtable.contains(key));
    }
    EXPECT_EQ(N/2, memtable.size());

    // iter
    iter.SeekToFirst();
    for(int i = 1; i < N; i += 2){
        std::string key(i, 'a');
        std::string value(i, 'b');
        EXPECT_EQ(true, iter.Valid());
        EXPECT_EQ(key, iter.key());
        EXPECT_EQ(value, iter.value());
        iter.Next();
    }
    iter.SeekToLast();
       for(int i = N - 1; i >= 0; i -= 2){
        std::string key(i, 'a');
        std::string value(i, 'b');
        EXPECT_EQ(true, iter.Valid());
        EXPECT_EQ(key, iter.key());
        EXPECT_EQ(value, iter.value());
        iter.Prev();
    }
}

TEST(SKIPLIST_TEST, large_test){
    const int N = 1024 * 16;
    
    // put and get
    for(int i = 0; i < N; i++){
        std::string key(i, 'a');
        std::string value(i, 'b');
        memtable.put(key, value);
        EXPECT_EQ(true, memtable.contains(key));
        EXPECT_EQ(std::pair(true, value), memtable.get(key));
    }
    EXPECT_EQ(N, memtable.size());

    // iter
    iter.SeekToFirst();
    for(int i = 0; i < N; i++){
        std::string key(i, 'a');
        std::string value(i, 'b');
        EXPECT_EQ(true, iter.Valid());
        EXPECT_EQ(key, iter.key());
        EXPECT_EQ(value, iter.value());
        iter.Next();
    }
    iter.SeekToLast();
       for(int i = N - 1; i >= 0; i--){
        std::string key(i, 'a');
        std::string value(i, 'b');
        EXPECT_EQ(true, iter.Valid());
        EXPECT_EQ(key, iter.key());
        EXPECT_EQ(value, iter.value());
        iter.Prev();
    }


    // 
    // del
    for(int i = 0; i < N; i+=2){
        std::string key(i, 'a');
        EXPECT_EQ(true, memtable.del(key));
        EXPECT_EQ(false, memtable.contains(key));
    }
    EXPECT_EQ(N/2, memtable.size());

    // iter
    iter.SeekToFirst();
    for(int i = 1; i < N; i += 2){
        std::string key(i, 'a');
        std::string value(i, 'b');
        EXPECT_EQ(true, iter.Valid());
        EXPECT_EQ(key, iter.key());
        EXPECT_EQ(value, iter.value());
        iter.Next();
    }
    iter.SeekToLast();
       for(int i = N - 1; i >= 0; i -= 2){
        std::string key(i, 'a');
        std::string value(i, 'b');
        EXPECT_EQ(true, iter.Valid());
        EXPECT_EQ(key, iter.key());
        EXPECT_EQ(value, iter.value());
        iter.Prev();
    }
}

int main(int argc, char **argv) {
  printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();   
}