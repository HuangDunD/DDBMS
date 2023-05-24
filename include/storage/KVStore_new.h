#pragma once

#include "KVStore_beta.h"
#include "Transaction.h"
#include "log_manager.h"

class KVStore : public KVStore_beta{
public:
    explicit KVStore(const std::string &dir, LogManager* log_manager)
        : KVStore_beta(dir), log_manager_(log_manager){};
    
    ~KVStore(){ flush(); };

    using KVStore_beta::put;
	using KVStore_beta::del;

    // put(key, value)
    void put(const std::string & key, const std::string &value, Transaction *txn, bool add_writeset = true){
        if(enable_logging){
            //写Put日志
            LogRecord record (txn->get_txn_id(), txn->get_prev_lsn(), LogRecordType::INSERT,
                        key.size(), key.c_str() ,value.size(), value.c_str());
            auto lsn = log_manager_->AppendLogRecord(record);
            txn->set_prev_lsn(lsn);
        }
        //add write record into write set.
        if(add_writeset){
            WriteRecord wr = WriteRecord(key, WType::INSERT_TUPLE);
            txn->get_write_set()->push_back(wr);
        }
        KVStore_beta::put(key, value);
    }

    // del(key)
    bool del(const std::string & key, Transaction *txn, bool add_writeset = true){
        std::unique_lock<std::mutex> l(mutex_);
        bool if_in_mem = memtable_.contains(key);
        if(if_in_mem){
            if(enable_logging){
                //写Del日志
                LogRecord record (txn->get_txn_id(), txn->get_prev_lsn(), LogRecordType::DELETE,
                            key.size(), key.c_str() ,memtable_.get(key).second.size(), memtable_.get(key).second.c_str());
                auto lsn = log_manager_->AppendLogRecord(record);
                txn->set_prev_lsn(lsn);
            }
            //add write record into write set.
            if(add_writeset){
                WriteRecord wr = WriteRecord(key, memtable_.get(key).second, WType::DELETE_TUPLE);
                txn->get_write_set()->push_back(wr);
            }
            memtable_.del(key);
            return true;
        }
        auto result = diskstorage_.search(key);
        if(result.first && result.second != ""){
            if(enable_logging){
                //写Del日志
                LogRecord record (txn->get_txn_id(), txn->get_prev_lsn(), LogRecordType::DELETE,
                            key.size(), key.c_str() , result.second.size(), result.second.c_str());
                auto lsn = log_manager_->AppendLogRecord(record);
                txn->set_prev_lsn(lsn);
            }
            //add write record into write set.
            if(add_writeset){
                WriteRecord wr = WriteRecord(key, result.second, WType::DELETE_TUPLE);
                txn->get_write_set()->push_back(wr);
            }
            memtable_.put(key, "");
            return true;
        }else{
            return false;
        }  
    }

private:
    LogManager *log_manager_;
};