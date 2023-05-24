#include "transaction_manager.h"
#include "transaction_manager_rpc.h"
#include "benchmark_config.h"

#include "gtest/gtest.h"
#include <filesystem>

class DataTest : public ::testing::Test {
   public:
    std::unique_ptr<KVStore> kv_;

   public:
    void SetUp() override {
        ::testing::Test::SetUp();
        kv_ = std::make_unique<KVStore>(FLAGS_DIR, nullptr);
    }
};

TEST_F(DataTest, workload){
    ASSERT_EQ("","");
    ASSERT_EQ(kv_->get("key1").second, "value1");
    ASSERT_EQ(kv_->get("key1000").second, "value1000");
    ASSERT_EQ(kv_->get("key3000").second, "value3000");
    ASSERT_EQ(kv_->get("key7000").second, "value7000");
    ASSERT_EQ(kv_->get("key9999").second, "value9999");
}

int main(int argc, char **argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    testing::InitGoogleTest(&argc, argv);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return RUN_ALL_TESTS();
}