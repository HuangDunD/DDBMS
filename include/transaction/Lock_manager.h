#pragma once

#include "txn.h"
#include "Transaction.h"
#include <mutex>
#include <condition_variable>  // NOLINT
#include <list>
#include <unordered_map>
#include <atomic>
#include <memory>
class Lock_manager
{
public:

    class LockRequest {
    public:
        LockRequest(txn_id_t txn_id, LockMode lock_mode)
            : txn_id_(txn_id), lock_mode_(lock_mode){}

        txn_id_t txn_id_;
        LockMode lock_mode_;
        bool granted_ = false;
    };

    class LockRequestQueue {
    public:
        /** List of lock requests for the same resource (table, partition or row) */
        std::list<LockRequest *> request_queue_;
        /** For notifying blocked transactions on this rid */
        std::condition_variable cv_;
        // waiting bit: if there is a lock request which is not granted, the waiting -bit will be true
        bool is_waiting_ = false;
        // upgrading_flag: if there is a lock waiting for upgrading, other transactions that request for upgrading will be aborted
        // (deadlock prevetion)
        bool upgrading_ = false;                    // 当前队列中是否存在一个正在upgrade的锁
        /** coordination */
        std::mutex latch_;
    };


public:
    Lock_manager(){
        std::atomic_init(&enable_no_wait_,false);
    };

    explicit Lock_manager(bool enable_no_wait) {
        std::atomic_init(&enable_no_wait_,enable_no_wait);
    }

    ~Lock_manager(){};

    /// @param txn 事务指针
    /// @param lock_mode 锁模式：读写、写锁、意向读锁、意向写锁、意象读写锁
    /// @param oid 表id
    /// @param p_id 分区id
    /// @param row_id 行id
    /// @return 加锁是否成功，成功返回true，否则返回false
    /// @brief 事务申请表锁
    auto LockTable(Transaction *txn, LockMode lock_mode, const table_oid_t &oid) -> bool;
    /// @brief 事务释放表锁
    auto UnLockTable(Transaction *txn, const table_oid_t &oid) -> bool;
    /// @brief 事务申请分区锁
    auto LockPartition(Transaction *txn, LockMode lock_mode, const table_oid_t &oid, const partition_id_t &p_id) -> bool;
    /// @brief 事务释放分区锁
    auto UnLockPartition(Transaction *txn, const table_oid_t &oid, const partition_id_t &p_id) -> bool;
    /// @brief 事务申请行锁
    auto LockRow(Transaction *txn, LockMode lock_mode, const table_oid_t &oid, const partition_id_t &p_id, const row_id_t &row_id) -> bool;
    /// @brief 事务释放行锁
    auto UnLockRow(Transaction *txn,  const table_oid_t &oid, const partition_id_t &p_id, const row_id_t &row_id) -> bool;


private:
    auto Unlock(Transaction *txn, const Lock_data_id &l_id ) -> bool;
    static auto isLockCompatible(const LockRequest *lock_request, const LockMode &target_lock_mode) -> bool;
    static auto isUpdateCompatible(const LockRequest *lock_request, const LockMode &upgrade_lock_mode) -> bool; 
    static auto checkSameTxnLockRequest(Transaction *txn, LockRequestQueue *request_queue, const LockMode targrt_lock_mode, std::unique_lock<std::mutex> &queue_lock) -> int;
    static auto checkQueueCompatible(const LockRequestQueue *request_queue, const LockRequest *request) -> bool;

    std::mutex latch_;  // 锁表的互斥锁，用于锁表的互斥访问
    std::unordered_map<Lock_data_id, LockRequestQueue> lock_map_;  //可上锁的数据(表,分区,行)数据与锁请求队列的对应关系

    static std::atomic<bool> enable_no_wait_ ;

};
