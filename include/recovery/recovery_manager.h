#ifndef RECOVERY_MANAGER_H
#define RECOVERY_MANAGER_H

#include "log_manager.h"
#include "log_record.h"
// #include "KVStore.h"
// #include "Inmemory/KVStore.h"
#include "KVStore_new.h"

#include <unordered_map>


class RecoveryManager {
public:
    RecoveryManager(LogStorage *log_storage, LogManager *log_manager)
        : log_storage_(log_storage), log_manager_(log_manager) {
        buffer_ = new char[LOG_BUFFER_SIZE];
    }

    ~RecoveryManager() {
        delete[] buffer_;
    }
    
    /**
     * @brief 
     * Perform the recovery procedure
     */
    void ARIES();

private:
    // helper function
    void Scan();
    void Redo();
    void Undo();
    void RedoLog(LogRecord &log_record);
    void UndoLog(LogRecord &log_record);

    // buffer used to store log data
    char *buffer_{nullptr};

    LogStorage *log_storage_;
    KVStore *kv_;
    LogManager *log_manager_;
    // we need to keep track of what txn we need to undo
    // txn -> last lsn
    std::unordered_map<txn_id_t, lsn_t> active_txn_;
    // remember the offset of a log record
    // lsn -> (offset in disk, size of log)
    std::unordered_map<lsn_t, std::pair<int, int>> lsn_mapping_;
};

#endif