#include <string>

#include "BlockBuilder.h"
#include "Block.h"
#include "Option.h"
#include "gtest/gtest.h"

BlockBuilder block_builder;

TEST(BlockBuilderTest, empty_test) {
    EXPECT_EQ(true, block_builder.empty());
    EXPECT_EQ(sizeof(uint64_t) * 3, block_builder.estimated_size());
}


TEST(BlockBuilderTest, simple_test) { 
    const int N = 1024;

    block_builder.reset();

    // block builder test
    uint64_t expect_size = sizeof(uint64_t) * 2;
    for(int i = 0; i < N; i++) {
        if(i % Option::RESTART_INTERVAL == 0) {
            expect_size += sizeof(uint64_t);
        }
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        expect_size += key.size() + value.size() + sizeof(uint64_t) * 2;
        
        block_builder.add(key, value);

        EXPECT_EQ(expect_size, block_builder.estimated_size());
    }

    // block phase
    Block block(block_builder.finish());
    
    // get接口测试
    for(int i = 0; i < N; i++) {
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        EXPECT_EQ(std::make_pair(true, value), block.get(key));
    }
    // 迭代器测试
    // 初始化
    std::unique_ptr<Iterator> iter = block.NewIterator();
    EXPECT_EQ(false, iter->Valid());
    // 测试接口SeekToFirst()和Next();
    iter->SeekToFirst();
    for(int i = 0; i < N; i++) {
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        EXPECT_EQ(true, iter->Valid());
        EXPECT_EQ(key, iter->Key());
        EXPECT_EQ(value, iter->Value());
        iter->Next();
    }
    EXPECT_EQ(false, iter->Valid());
    // 测试SeekToLast()
    iter->SeekToLast();
    EXPECT_EQ(true, iter->Valid());
    EXPECT_EQ(std::string(N, 'a'), iter->Key());
    EXPECT_EQ(std::string(N, 'b'), iter->Value());
    // 测试Seek
    for(int i = 0; i < N; i++) {
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        iter->Seek(key);
        EXPECT_EQ(true, iter->Valid());
        EXPECT_EQ(key, iter->Key());
        EXPECT_EQ(value, iter->Value());
        iter->Next();
    }
}

TEST(BlockBuilderTest, repeat_test) { 
    block_builder.reset();
    const int N = 1024;
    uint64_t expect_size = sizeof(uint64_t) * 2;
    for(int i = 0; i < N; i++) {
        if(i % Option::RESTART_INTERVAL == 0) {
            expect_size += sizeof(uint64_t);
        }
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        expect_size += key.size() + value.size() + sizeof(uint64_t) * 2;
        
        block_builder.add(key, value);

        EXPECT_EQ(expect_size, block_builder.estimated_size());
    }
}

TEST(BlockBuilderTest, large_test) { 
    const int N = 1024 * 4;

    block_builder.reset();

    // block builder test
    uint64_t expect_size = sizeof(uint64_t) * 2;
    for(int i = 0; i < N; i++) {
        if(i % Option::RESTART_INTERVAL == 0) {
            expect_size += sizeof(uint64_t);
        }
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        expect_size += key.size() + value.size() + sizeof(uint64_t) * 2;
        
        block_builder.add(key, value);

        EXPECT_EQ(expect_size, block_builder.estimated_size());
    }

    // block phase
    Block block(block_builder.finish());
    
    // get接口测试
    for(int i = 0; i < N; i++) {
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        EXPECT_EQ(std::make_pair(true, value), block.get(key));
    }
    // 迭代器测试
    // 初始化
    std::unique_ptr<Iterator> iter = block.NewIterator();
    EXPECT_EQ(false, iter->Valid());
    // 测试接口SeekToFirst()和Next();
    iter->SeekToFirst();
    for(int i = 0; i < N; i++) {
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        EXPECT_EQ(true, iter->Valid());
        EXPECT_EQ(key, iter->Key());
        EXPECT_EQ(value, iter->Value());
        iter->Next();
    }
    EXPECT_EQ(false, iter->Valid());
    // 测试SeekToLast()
    iter->SeekToLast();
    EXPECT_EQ(true, iter->Valid());
    EXPECT_EQ(std::string(N, 'a'), iter->Key());
    EXPECT_EQ(std::string(N, 'b'), iter->Value());
    // 测试Seek
    for(int i = 0; i < N; i++) {
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        iter->Seek(key);
        EXPECT_EQ(true, iter->Valid());
        EXPECT_EQ(key, iter->Key());
        EXPECT_EQ(value, iter->Value());
        iter->Next();
    }
}

int main(int argc, char **argv) {
  printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();   
}