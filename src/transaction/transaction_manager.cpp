#include <brpc/channel.h>
#include <butil/logging.h>
#include <future>
#include <functional>

#include "transaction_manager.h"
#include "transaction_manager.pb.h"
#include "dbconfig.h"

std::unordered_map<txn_id_t, Transaction *> TransactionManager::txn_map = {};
std::shared_mutex TransactionManager::txn_map_mutex = {};
   
uint64_t TransactionManager::getTimestampFromServer(){
    brpc::Channel channel;
    brpc::ChannelOptions options;
    options.timeout_ms = 1000;
    options.max_retry = 3;

    if (channel.Init(FLAGS_META_SERVER_ADDR.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        return -1;
    }
    meta_service::MetaService_Stub stub(&channel);
    brpc::Controller cntl;
    meta_service::getTimeStampRequest request;
    meta_service::getTimeStampResponse response;
    stub.GetTimeStamp(&cntl, &request, &response, NULL);

    if(cntl.Failed()) {
        LOG(WARNING) << cntl.ErrorText();
    }
    return response.timestamp();
}

void TransactionManager::ReleaseLocks(Transaction *txn){

    for(auto iter = txn->get_row_S_lock_set()->begin(); iter != txn->get_row_S_lock_set()->end();){
        auto o_id = iter->oid_;
        auto p_id = iter->p_id_;
        auto r_id = iter->row_id_;
        auto next_iter = ++iter;
        lock_manager_->UnLockRow(txn, o_id, p_id, r_id);
        iter = next_iter;
    }
    for(auto iter = txn->get_row_X_lock_set()->begin(); iter != txn->get_row_X_lock_set()->end();){
        auto o_id = iter->oid_;
        auto p_id = iter->p_id_;
        auto r_id = iter->row_id_;
        auto next_iter = ++iter;
        lock_manager_->UnLockRow(txn, o_id, p_id, r_id);
        iter = next_iter;
    }

    for(auto iter = txn->get_partition_S_lock_set()->begin(); iter != txn->get_partition_S_lock_set()->end();){
        auto o_id = iter->oid_;
        auto p_id = iter->p_id_;
        auto next_iter = ++iter;
        lock_manager_->UnLockPartition(txn, o_id, p_id);
        iter = next_iter;
    }
    for(auto iter = txn->get_partition_IS_lock_set()->begin(); iter != txn->get_partition_IS_lock_set()->end();){
        auto o_id = iter->oid_;
        auto p_id = iter->p_id_;
        auto next_iter = ++iter;
        lock_manager_->UnLockPartition(txn, o_id, p_id);
        iter = next_iter;
    }
    for(auto iter = txn->get_partition_IX_lock_set()->begin(); iter != txn->get_partition_IX_lock_set()->end();){
        auto o_id = iter->oid_;
        auto p_id = iter->p_id_;
        auto next_iter = ++iter;
        lock_manager_->UnLockPartition(txn, o_id, p_id);
        iter = next_iter;
    }
    for(auto iter = txn->get_partition_SIX_lock_set()->begin(); iter != txn->get_partition_SIX_lock_set()->end();){
        auto o_id = iter->oid_;
        auto p_id = iter->p_id_;
        auto next_iter = ++iter;
        lock_manager_->UnLockPartition(txn, o_id, p_id);
        iter = next_iter;
    }
    for(auto iter = txn->get_partition_X_lock_set()->begin(); iter != txn->get_partition_X_lock_set()->end();){
        auto o_id = iter->oid_;
        auto p_id = iter->p_id_;
        auto next_iter = ++iter;
        lock_manager_->UnLockPartition(txn, o_id, p_id);
        iter = next_iter;
    }

    for(auto iter = txn->get_table_S_lock_set()->begin(); iter != txn->get_table_S_lock_set()->end();){
        auto o_id = iter->oid_;
        auto next_iter = ++iter;
        lock_manager_->UnLockTable(txn, o_id);
        iter = next_iter;
    }
    for(auto iter = txn->get_table_IS_lock_set()->begin(); iter != txn->get_table_IS_lock_set()->end();){
        auto o_id = iter->oid_;
        auto next_iter = ++iter;
        lock_manager_->UnLockTable(txn, o_id);
        iter = next_iter;
    }
    for(auto iter = txn->get_table_IX_lock_set()->begin(); iter != txn->get_table_IX_lock_set()->end();){
        auto o_id = iter->oid_;
        auto next_iter = ++iter;
        lock_manager_->UnLockTable(txn, o_id);
        iter = next_iter;
    }
    for(auto iter = txn->get_table_SIX_lock_set()->begin(); iter != txn->get_table_SIX_lock_set()->end();){
        auto o_id = iter->oid_;
        auto next_iter = ++iter;
        lock_manager_->UnLockTable(txn, o_id);
        iter = next_iter;
    }
    for(auto iter = txn->get_table_X_lock_set()->begin(); iter != txn->get_table_X_lock_set()->end();){
        auto o_id = iter->oid_;
        auto next_iter = ++iter;
        lock_manager_->UnLockTable(txn, o_id);
        iter = next_iter;
    }
    
    return ;
}

Transaction* TransactionManager::Begin(Transaction*& txn, txn_id_t txn_id, IsolationLevel isolation_level){
    global_txn_latch_.lock_shared();

    if (txn == nullptr) {
        txn = new Transaction(txn_id, isolation_level); 
    }

    if (enable_logging) {
        LogRecord record(txn->get_txn_id(), txn->get_prev_lsn(), LogRecordType::BEGIN);
        lsn_t lsn = log_manager_->AppendLogRecord(record);
        txn->set_prev_lsn(lsn);
    }

    std::unique_lock<std::shared_mutex> l(txn_map_mutex);
    txn_map[txn->get_txn_id()] = txn;
    return txn;
}

Transaction* TransactionManager::Begin(Transaction*& txn, IsolationLevel isolation_level){

    // 1. 判断传入事务参数是否为空指针
    // 2. 如果为空指针，创建新事务
    // 3. 把开始事务加入到全局事务表中
    // 4. 返回当前事务指针
    global_txn_latch_.lock_shared();

    if (txn == nullptr) {
        txn = new Transaction(getTimestampFromServer(), isolation_level); 
    }

    if (enable_logging) {
        LogRecord record(txn->get_txn_id(), txn->get_prev_lsn(), LogRecordType::BEGIN);
        lsn_t lsn = log_manager_->AppendLogRecord(record);
        txn->set_prev_lsn(lsn);
    }

    std::unique_lock<std::shared_mutex> l(txn_map_mutex);
    txn_map[txn->get_txn_id()] = txn;
    return txn;
}

bool TransactionManager::AbortSingle(Transaction * txn){
    // 1. 回滚事务写集
    // 2. 写Abort日志
    // 3. 释放锁
    // 4. 从事务图中移除事务
    return true;
}

bool TransactionManager::CommitSingle(Transaction * txn){
    // 1. 回滚事务写集
    // 2. 写Commit日志
    // 3. 释放锁
    // 4. 从事务图中移除事务

    return true;
}

bool TransactionManager::Abort(Transaction * txn){
    // 回滚一个事务的函数入口
    // 考虑以下几种情况, txn是否是分布式事务, 
    // 如果不是分布式事务，事务的执行是否在本节点上？
    // 如果非分布式事务但执行不在本节点，则需要RPC调用回滚，否则可以直接调用AbortSingle
    // 如果是分布式事务，则需要向所有参与者发送回滚请求
    
    return true;
}

bool TransactionManager::Commit(Transaction * txn){
    // 提交一个事务的函数入口
    // 类似于Abort()，也需要考虑txn的几种情况
    // 如果事务是分布式事务则需要发起两阶段提交请求
    
}

bool TransactionManager::PrepareCommit(Transaction * txn){
    // 参与者接收协调者发起的准备提交请求，
    // 查看此时事务状态并决定是否可以提交，
    // 修改事务状态，写入prepare日志
    return true;
}