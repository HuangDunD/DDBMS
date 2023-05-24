#include "recovery_manager.h"
#include <set>

void RecoveryManager::ARIES() {
    Scan();
    Redo();
    Undo();
}

void RecoveryManager::Scan() {
    // only perform scan(analyse) phase when we need to recover from checkpoint
}

void RecoveryManager::Redo() {
    // currently, we only support recovery from empty database, so there is no dirty page
    int offset = 0;
    int max_lsn = INVALID_LSN;
    while (log_storage_->ReadLog(buffer_, LOG_BUFFER_SIZE, offset)) {
        int inner_offset = 0;
        while (true) {
            // first probe the size
            uint32_t size = *reinterpret_cast<const uint32_t *>(buffer_ + inner_offset);
            // size = 0 means there is no more log records
            // we shall stop when buffer is empty or there is no more log records
            if (size == 0 || size + inner_offset > LOG_BUFFER_SIZE) {
                break;
            }
            auto log = LogRecord::DeserializeFrom(buffer_ + inner_offset);
            // update max lsn
            max_lsn = std::max(max_lsn, log.GetLsn());
            // remember the necessary information to retrieve log based on lsn
            lsn_mapping_[log.GetLsn()] = std::make_pair(offset + inner_offset, size);
            // redo the log if necessary
            RedoLog(log);

            inner_offset += size;
        }
        offset += inner_offset;
    }
    log_manager_->SetNextLsn(max_lsn + 1);
}

void RecoveryManager::RedoLog(LogRecord &log_record) {
    // record last lsn
    active_txn_[log_record.GetTxnId()] = log_record.GetLsn();

    switch (log_record.GetLogRecordType()) {
    case LogRecordType::COMMIT:
    case LogRecordType::ABORT:
        active_txn_.erase(log_record.GetTxnId());
        break;
    case LogRecordType::BEGIN:
        active_txn_[log_record.GetTxnId()] = log_record.GetLsn();
        break;
    case LogRecordType::INSERT: {
        kv_->put(std::string(log_record.GetKey(), log_record.GetKeySize()),
            std::string(log_record.GetValue(), log_record.GetValueSize()));
        break;
    }
    case LogRecordType::DELETE: {
        kv_->del(std::string(log_record.GetKey(), log_record.GetKeySize()));
        break;
    }
    case LogRecordType::CREATE_TABLE: 
    case LogRecordType::DROP_TABLE:
    case LogRecordType::PREPARED:{
        // TODO
        // do nothing now
        break;
    }
    default:
        std::cerr <<  "Invalid Log Type";
    }
}

void RecoveryManager::Undo() {
    // abort all the active transactions
    // at every step in undo phase, we need to execute the log record with max lsn in undo-list
    std::set<lsn_t> next_lsn;
    for (auto &[key, lsn]: active_txn_) {
        next_lsn.insert(lsn);
    }

    while (!next_lsn.empty()) {
        auto lsn = *next_lsn.rbegin();
        // first fetch the offset and size
        auto [offset, size] = lsn_mapping_[lsn];
        log_storage_->ReadLog(buffer_, size, offset);
        // deserialize the log
        auto log = LogRecord::DeserializeFrom(buffer_);
        // undo log
        UndoLog(log);
        // erase current lsn and insert the previous lsn
        next_lsn.erase(lsn);
        if (log.GetPrevLsn() != INVALID_LSN) {
            next_lsn.insert(log.GetPrevLsn());
        }
    }
    // after redo phase is done, write abort record
    for (auto &[txn_id, lsn]: active_txn_) {
        auto log = LogRecord(txn_id, lsn, LogRecordType::ABORT);
        log_manager_->AppendLogRecord(log);
    }
}

void RecoveryManager::UndoLog(LogRecord &log_record) {
    switch (log_record.GetLogRecordType()) {
    case LogRecordType::BEGIN:
    case LogRecordType::PREPARED:
        // do nothing
        break;
    case LogRecordType::INSERT: {
        // 这里调用Kvstore不传入txn事务, 因为在恢复过程中没有维护事务管理器, 通过active_txn_来跟踪
        // 每个事务最后的lsn, 写入存储层后, 在本函数中通过active_txn_维护写入的日志信息
        kv_->del(std::string(log_record.GetKey(),log_record.GetKeySize()));
        // undo操作也需要写redo log
        auto log = LogRecord(log_record.GetTxnId(), 
                             active_txn_[log_record.GetTxnId()],  
                             LogRecordType::DELETE, 
                             log_record.GetKeySize(), log_record.GetKey(),
                             log_record.GetValueSize(), log_record.GetValue());
        log_manager_->AppendLogRecord(log);

        // update last lsn
        active_txn_[log_record.GetTxnId()] = log.GetLsn();
        break;
    }
    case LogRecordType::DELETE: {
        kv_->put(std::string(log_record.GetKey(),log_record.GetKeySize()),
                    std::string(log_record.GetKey(),log_record.GetKeySize()));

        auto log = LogRecord(log_record.GetTxnId(), 
                             active_txn_[log_record.GetTxnId()],  
                             LogRecordType::INSERT, 
                             log_record.GetKeySize(), log_record.GetKey(),
                             log_record.GetValueSize(), log_record.GetValue());
        log_manager_->AppendLogRecord(log);
        // update lsn
        active_txn_[log_record.GetTxnId()] = log.GetLsn();
        break;
    }
    default:
        std::cerr <<  "Invalid Log Type";
    }

}