#include <gtest/gtest.h>
#include "log_manager.h"

class LogManagerTest : public testing::Test {
protected:
    LogManagerTest() {
        // 每个测试用例都会创建一个 LogManager 实例
        LogStorage *log_storage = new LogStorage("test"); 
        log_manager_ = new LogManager(log_storage);
    }
    ~LogManagerTest() override {
        delete log_manager_;
    }
    LogManager *log_manager_;
};

TEST_F(LogManagerTest, AppendLogRecordTest) {
    // 测试用例 1
    LogRecord record1(1, 0, LogRecordType::INSERT, 4, "key1", 6, "value1");
    lsn_t lsn1 = log_manager_->AppendLogRecord(record1);
    ASSERT_EQ(lsn1, 0); // 第一条日志的 LSN 应该为 0
    EXPECT_EQ(log_manager_->GetFlushLsn(), -1); // 尚未执行 flush 操作，因此 flushed LSN 应该为 -1
    EXPECT_EQ(log_manager_->GetPersistentLsn(), -1); // 目前已经写入的最后一条日志的 LSN 应该为 0
    std::this_thread::sleep_for(std::chrono::milliseconds(35));
    EXPECT_EQ(log_manager_->GetPersistentLsn(), 0); // 执行 flush 操作，因此 flushed LSN 应该为 0

    // 测试用例 2
    LogRecord record2(1, 0, LogRecordType::DELETE, 4, "key2", 6, "value2");
    lsn_t lsn2 = log_manager_->AppendLogRecord(record2);
    ASSERT_EQ(lsn2, 1); // 第二条日志的 LSN 应该为 1
    std::this_thread::sleep_for(std::chrono::milliseconds(35));
    EXPECT_EQ(log_manager_->GetPersistentLsn(), 1); 

    // 测试用例 3
    LogRecord record3(1, 0, LogRecordType::INSERT, 4, "key3", 6, "value3");
    lsn_t lsn3 = log_manager_->AppendLogRecord(record3);
    ASSERT_EQ(lsn3, 2); // 第三条日志的 LSN 应该为 2
    std::this_thread::sleep_for(std::chrono::milliseconds(35));
    EXPECT_EQ(log_manager_->GetPersistentLsn(), 2); 
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    // LogRecord *record1 = new LogRecord(1, 0, LogRecordType::INSERT, 4, "key1", 6, "value1");
    // lsn_t lsn1 = log_manager_->AppendLogRecord(record1);
}
