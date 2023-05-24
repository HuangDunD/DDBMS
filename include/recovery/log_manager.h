#pragma once
#include "log_record.h"
#include "storage/LogStorage.h"
#include "dbconfig.h"
#include <chrono>
#include <string.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

static constexpr int BUFFER_POOL_SIZE = 10;                                          // size of buffer pool
static constexpr int PAGE_SIZE = 4096;                                        // size of a data page in byte
static constexpr int LOG_BUFFER_SIZE = ((BUFFER_POOL_SIZE + 1) * PAGE_SIZE);  // size of a log buffer in byte
static constexpr auto LOG_TIMEOUT = std::chrono::milliseconds(30);

class LogManager
{
public:
    explicit LogManager(LogStorage *log_storage)
      :  enable_flushing_(true), needFlush_(false), next_lsn_(0), persistent_lsn_(INVALID_LSN), flush_lsn_(INVALID_LSN), log_storage_(log_storage) {
        log_buffer_ = new char[LOG_BUFFER_SIZE];
        flush_buffer_ = new char[LOG_BUFFER_SIZE];
        RunFlushThread();
    }

    ~LogManager() {
        StopFlushThread();
        delete[] log_buffer_;
        delete[] flush_buffer_;
        log_buffer_ = nullptr;
        flush_buffer_ = nullptr;
    }

    void RunFlushThread();

    void StopFlushThread(){ 
        enable_flushing_.store(false); 
        flush_thread_->join(); 
        delete flush_thread_;
    }
    
    void Flush(lsn_t lsn, bool force);

    lsn_t AppendLogRecord(LogRecord &log_record);

    void SwapBuffer();

    inline lsn_t GetNextLsn() const { return next_lsn_; }
    inline void SetNextLsn(lsn_t next_lsn) {next_lsn_ = next_lsn;}
    inline lsn_t GetFlushLsn() const { return flush_lsn_; }
    inline char * GetLogBuffer() const { return log_buffer_; }
    inline lsn_t GetPersistentLsn() const { return persistent_lsn_; }

private:

    char *log_buffer_; // 用来暂时存储系统运行过程中添加的日志; append log_record into log_buffer
    char *flush_buffer_; // 用来暂时存储需要刷新到磁盘中的日志; flush the logs in flush_buffer into disk file

    std::atomic<bool> enable_flushing_; //是否允许刷新
    std::atomic<bool> needFlush_; //是否需要刷盘
    std::atomic<lsn_t> next_lsn_; // 用于分发日志序列号; next lsn in the system
    std::atomic<lsn_t> persistent_lsn_; // 已经刷新到磁盘中的最后一条日志的日志序列号; the last persistent lsn
    lsn_t flush_lsn_; // flush_buffer_中最后一条日志的日志记录号; the last lsn in the flush_buffer

    size_t log_buffer_write_offset_ = 0; // log_buffer_的偏移量
    size_t flush_buffer_write_offset_ = 0; // flush_buffer_的偏移量

    std::thread *flush_thread_; // 日志刷新线程

    std::mutex latch_{}; // 互斥锁，用于log_buffer_的互斥访问

    std::condition_variable cv_; // 条件变量，用于flush_thread的唤醒; to notify the flush_thread
    
    std::condition_variable operation_cv_;

    LogStorage *log_storage_;
};
