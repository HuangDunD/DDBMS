#include "transaction_manager.h"
#include "transaction_manager_rpc.h"
#include "benchmark_config.h"

#include "gtest/gtest.h"
#include <filesystem>

class SetWorkloadTest : public ::testing::Test {
   public:
    std::unique_ptr<KVStore> kv_;

   public:
    void SetUp() override {
        ::testing::Test::SetUp();
        if(std::filesystem::exists(FLAGS_DIR)) {
            std::filesystem::remove_all(FLAGS_DIR);
            std::filesystem::remove(FLAGS_DIR);
        }

        kv_ = std::make_unique<KVStore>(FLAGS_DIR, nullptr);

        enable_logging = false; // 关闭日志
        for (int i=0; i < FLAGS_BANCHMARK_NUM; i++){
            kv_->put("key"+std::to_string(i), "value"+std::to_string(i));
        }
        kv_->flush();
        enable_logging = true; // 打开日志
    }
};

TEST_F(SetWorkloadTest, workload){
    ASSERT_EQ("","");
    ASSERT_EQ(kv_->get("key1").second, "value1");
    ASSERT_EQ(kv_->get("key1000").second, "value1000");
    ASSERT_EQ(kv_->get("key3000").second, "value3000");
    ASSERT_EQ(kv_->get("key7000").second, "value7000");
    ASSERT_EQ(kv_->get("key9999").second, "value9999");
}

int main(int argc, char **argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::cout << "正在导入数据..." << std::endl;
    testing::InitGoogleTest(&argc, argv);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "导入完成." << std::endl;
    return RUN_ALL_TESTS();
}