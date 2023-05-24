#include "log_manager.h"
#include <iostream>

lsn_t LogManager::AppendLogRecord(LogRecord &log_record) {
    std::unique_lock<std::mutex> l(latch_);
    // for debug
    // std::cout << "append log record: " << static_cast<int>(log_record.GetLogRecordType()) << std::endl;

    if (log_buffer_write_offset_ + log_record.GetSize() >= LOG_BUFFER_SIZE) {
        needFlush_ = true;
        cv_.notify_one(); //let RunFlushThread wake up.
        operation_cv_.wait(l, [&] {return log_buffer_write_offset_ + log_record.GetSize()< LOG_BUFFER_SIZE;});
    }
    log_record.SetLsn(next_lsn_++);
    memcpy(log_buffer_ + log_buffer_write_offset_, &log_record, LogRecord::HEADER_SIZE);
    int pos = log_buffer_write_offset_ + LogRecord::HEADER_SIZE;

    if(log_record.GetLogRecordType() == LogRecordType::INSERT ||
            log_record.GetLogRecordType() == LogRecordType::DELETE ){
        memcpy(log_buffer_ + pos, &log_record.GetKeySize(), sizeof(uint32_t));
        pos += sizeof(uint32_t);
        memcpy(log_buffer_ + pos, log_record.GetKey(), log_record.GetKeySize());
        pos += log_record.GetKeySize();

        memcpy(log_buffer_ + pos, &log_record.GetValueSize(), sizeof(uint32_t));
        pos += sizeof(uint32_t);
        memcpy(log_buffer_ + pos, log_record.GetValue(), log_record.GetValueSize());
        pos += log_record.GetValueSize();
    }
    // else if(log_record.GetLogRecordType() == LogRecordType::UPDATE){
    //     memcpy(log_buffer_ + pos, &log_record.GetKeySize(), sizeof(uint32_t));
    //     pos += sizeof(uint32_t);
    //     memcpy(log_buffer_ + pos, log_record.GetKey(), log_record.GetKeySize());
    //     pos += log_record.GetKeySize();

    //     memcpy(log_buffer_ + pos, &log_record.GetValueSize(), sizeof(uint32_t));
    //     pos += sizeof(uint32_t);
    //     memcpy(log_buffer_ + pos, log_record.GetValue(), log_record.GetValueSize());
    //     pos += log_record.GetValueSize();

    //     memcpy(log_buffer_ + pos, &log_record.GetOldValueSize(), sizeof(uint32_t));
    //     pos += sizeof(uint32_t);
    //     memcpy(log_buffer_ + pos, log_record.GetOldValue(), log_record.GetOldValueSize());
    //     pos += log_record.GetOldValueSize();
    // }
    log_buffer_write_offset_ = pos;
    return log_record.GetLsn();
};

void LogManager::RunFlushThread(){
    enable_flushing_.store(true);
    flush_thread_ = new std::thread([&]{
        while (enable_flushing_) {
            std::unique_lock<std::mutex> l(latch_);
            //每隔LOG_TIMEOUT刷新一次或者buffer已满或者强制刷盘
            cv_.wait_for(l, LOG_TIMEOUT, [&] {return needFlush_.load();});

            if(log_buffer_write_offset_ > 0){
                // swap buffer
                SwapBuffer();
                lsn_t lsn = next_lsn_ - 1;
                flush_lsn_ = lsn;
                l.unlock();
                // resume the append log record operation 
                operation_cv_.notify_all();
                
                //flush log
                log_storage_->WriteLog(flush_buffer_, flush_buffer_write_offset_);
                persistent_lsn_.store(lsn);
                needFlush_.store(false);
            }
        }
    });
}

void LogManager::SwapBuffer() {
    // we are in the critical section, it's safe to exchange these two variable
    std::swap(log_buffer_, flush_buffer_);
    flush_buffer_write_offset_ =  log_buffer_write_offset_ ;
    log_buffer_write_offset_ = 0;
}

void LogManager::Flush(lsn_t lsn, bool force) {
    if (force) {
        needFlush_ = true;
        // notify flush thread to start flushing the log
        cv_.notify_one();
    }
    while (persistent_lsn_.load() < lsn) {}
}
