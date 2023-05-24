#include <iostream>
#include <mutex>
#include "Lock_manager.h"

std::atomic<bool> Lock_manager::enable_no_wait_;

//NOTE:
auto Lock_manager::isLockCompatible(const LockRequest *iter, const LockMode &target_lock_mode) -> bool {
        switch (target_lock_mode) {
            case LockMode::INTENTION_SHARED:
                if(iter->lock_mode_ == LockMode::EXLUCSIVE){
                    return false;
                }
                break;
            case LockMode::INTENTION_EXCLUSIVE:
                if(iter->lock_mode_ != LockMode::INTENTION_SHARED && iter->lock_mode_ != LockMode::INTENTION_EXCLUSIVE){
                    return false;
                }
                break;
            case LockMode::SHARED:
                if(iter->lock_mode_ != LockMode::INTENTION_SHARED && iter->lock_mode_ != LockMode::SHARED){
                    return false;
                }
                break;
            case LockMode::S_IX:
                if(iter->lock_mode_ != LockMode::INTENTION_SHARED) {
                    return false;
                }
                break;
            case LockMode::EXLUCSIVE:
                return false;
            default:
                return false;
            }
        return true;
}

auto Lock_manager::isUpdateCompatible(const LockRequest *iter, const LockMode &upgrade_lock_mode) -> bool {
    switch (iter->lock_mode_) {
        case LockMode::INTENTION_SHARED:
            if(upgrade_lock_mode == LockMode::INTENTION_SHARED){
                return false;
            }
            break;
        case LockMode::SHARED:
            if(upgrade_lock_mode != LockMode::EXLUCSIVE && upgrade_lock_mode != LockMode::S_IX){
                return false;
            }
            break;
        case LockMode::INTENTION_EXCLUSIVE:
            if(upgrade_lock_mode != LockMode::EXLUCSIVE && upgrade_lock_mode != LockMode::S_IX){
                return false;
            }
            break;
        case LockMode::S_IX:
            if(upgrade_lock_mode != LockMode::EXLUCSIVE){
                return false;
            }
            break;
        case LockMode::EXLUCSIVE:
            return false;
            break;
        default:
            return false;
        }
    return true;
}

auto Lock_manager::checkSameTxnLockRequest(Transaction *txn, LockRequestQueue *request_queue, const LockMode target_lock_mode, std::unique_lock<std::mutex> &queue_lock) -> int
{
    try{
        for(auto &iter : request_queue->request_queue_){
            if( iter->txn_id_ == txn->get_txn_id() ){
                //如果当前事务正在或已经申请同等模式的锁
                if(iter->lock_mode_ == target_lock_mode && iter->granted_ == true){
                    // 已经获取锁, 上锁成功
                    return true; 
                }
                else if(iter->lock_mode_ == target_lock_mode && iter->granted_ == false){
                    //正在申请锁, 等待这个请求申请成功后返回
                    request_queue->cv_.wait(queue_lock, [&request_queue, &iter, &txn]{
                        //TODO deadlock
                        if(Lock_manager::enable_no_wait_==false)
                            return Lock_manager::checkQueueCompatible(request_queue, iter) || 
                                txn->get_state()==TransactionState::ABORTED;
                        if(Lock_manager::checkQueueCompatible(request_queue,iter)==true)
                            return true;
                        else{
                            txn->set_transaction_state(TransactionState::ABORTED);
                            return true;
                        }
                    });
                    if(txn->get_state()==TransactionState::ABORTED) 
                        throw TransactionAbortException (txn->get_txn_id(), AbortReason::DEAD_LOCK_PREVENT_NO_WAIT) ;
                    iter->granted_ = true;
                    return true; 
                }
                //如果锁的模式不同, 则需要对锁进行升级, 首先检查是否有正在升级的锁, 如果有, 则返回升级冲突
                else if(request_queue->upgrading_ == true){
                    throw TransactionAbortException (txn->get_txn_id(), AbortReason::UPGRADE_CONFLICT);
                }
                else if(Lock_manager::isUpdateCompatible(iter, target_lock_mode) == false){
                    //如果没有冲突则检查是否锁兼容, 如果升级不兼容
                    throw TransactionAbortException (txn->get_txn_id(), AbortReason::INCOMPATIBLE_UPGRADE);
                }else{
                    //如果升级兼容, 则upgrade 
                    request_queue->upgrading_ = true; 
                    iter->lock_mode_ = target_lock_mode;
                    iter->granted_ = false;
                    request_queue->cv_.wait(queue_lock, [&request_queue, &iter, &txn]{
                        if(Lock_manager::enable_no_wait_==false)
                            return Lock_manager::checkQueueCompatible(request_queue, iter) || 
                                txn->get_state()==TransactionState::ABORTED;
                        // deadlock prevent
                        if(Lock_manager::checkQueueCompatible(request_queue,iter)==true)
                            return true;
                        else{
                            txn->set_transaction_state(TransactionState::ABORTED);
                            return true;
                        }
                    });
                    if(txn->get_state()==TransactionState::ABORTED) 
                        throw TransactionAbortException (txn->get_txn_id(), AbortReason::DEAD_LOCK_PREVENT_NO_WAIT) ;
                    request_queue->upgrading_ = false;
                    iter->granted_ = true;
                    
                    return true;
                } 
            }
        }
        return false;
    }
    catch(TransactionAbortException &e){
        txn->set_transaction_state(TransactionState::ABORTED);
        // std::cerr << e.GetInfo() << std::endl;
        return true;
    }
}

auto Lock_manager::checkQueueCompatible(const LockRequestQueue *request_queue, const LockRequest *request) -> bool {
    for(auto iter : request_queue->request_queue_ ){
        if(iter != request && iter->granted_ == true){
            if(isLockCompatible(iter, request->lock_mode_) == false)
                return false;
        }
    }
    return true;
}

auto Lock_manager::LockTable(Transaction *txn, LockMode lock_mode, const table_oid_t &oid) -> bool {
    
    try{
        //步骤一: 检查事务状态
        if(txn->get_state() == TransactionState::DEFAULT){
            txn->set_transaction_state(TransactionState::GROWING);
        } 
        if(txn->get_state() != TransactionState::GROWING){
            throw TransactionAbortException (txn->get_txn_id(), AbortReason::LOCK_ON_SHRINKING);
        }

        //步骤二: 得到l_id
        Lock_data_id l_id(oid, Lock_data_type::TABLE);

        //步骤三: 通过mutex申请全局锁表
        // latch_.lock();
        std::unique_lock<std::mutex> Latch(latch_);
        LockRequestQueue* request_queue = &lock_map_[l_id];
        std::unique_lock<std::mutex> queue_lock(request_queue->latch_);
        Latch.unlock();

        //步骤四: 
        //查找当前事务是否已经申请了目标数据项上的锁，
        //如果存在, 并且申请锁模式相同,返回申请成功
        //如果存在, 但与之锁的模式不同, 则准备升级, 并检查升级是否兼容
        auto target_lock_mode = lock_mode;
        if(checkSameTxnLockRequest(txn, request_queue, target_lock_mode, queue_lock)==true){
            if(txn->get_state() == TransactionState::ABORTED)
                return false;
            if(txn->get_lock_set(Lock_data_type::TABLE,target_lock_mode)->count(l_id)==0){
                txn->add_lock_set(target_lock_mode, l_id);
            }
            return true;
        }
        
        //步骤五, 如果当前事务在请求队列中没有申请该数据项的锁, 则新建请求加入队列
        //检查是否可以上锁, 否则阻塞, 使用条件变量cv来实现
        LockRequest* lock_request = new LockRequest(txn->get_txn_id(), target_lock_mode); 
        request_queue->request_queue_.emplace_back(lock_request); 
        
        // request_queue->cv_.wait(queue_lock, [&request_queue, &lock_request, &txn] {
        //                 return Lock_manager::checkQueueCompatible(request_queue, lock_request) || 
        //                     txn->get_state()==TransactionState::ABORTED;
        //             });

        request_queue->cv_.wait(queue_lock, [&request_queue, &lock_request, &txn]{
            // 如果不使用NO-Wait算法，则检查队列锁请求兼容情况和事务状态，
            // 如果可以锁请求兼容或事务已经回滚，则返回true，跳出等待
            if(Lock_manager::enable_no_wait_==false)
                return Lock_manager::checkQueueCompatible(request_queue, lock_request) || 
                    txn->get_state()==TransactionState::ABORTED;
            // 如果使用No-Wait算法， 如果当前请求与锁请求队列兼容
            // 那么返回true，跳出等待
            if(Lock_manager::checkQueueCompatible(request_queue,lock_request)==true)
                return true;
            // 否则，当前事务回滚，跳出等待
            else{
                txn->set_transaction_state(TransactionState::ABORTED);
                return true;
            }
        });
        if(txn->get_state()==TransactionState::ABORTED) 
            throw TransactionAbortException (txn->get_txn_id(), AbortReason::DEAD_LOCK_PREVENT_NO_WAIT) ;

        lock_request->granted_ = true;

        txn->add_lock_set(target_lock_mode, l_id); 

        return true;
    }
    catch(TransactionAbortException &e)
    {
        txn->set_transaction_state(TransactionState::ABORTED);
        std::cerr << e.GetInfo() << '\n';
        return false;
    }
}

auto Lock_manager::LockPartition(Transaction *txn, LockMode lock_mode, const table_oid_t &oid, const partition_id_t &p_id) -> bool {
    // for debug
    std::cout << oid << " " << p_id << std::endl;
    try{
        //检查事务状态
        if(txn->get_state() == TransactionState::DEFAULT){
            txn->set_transaction_state(TransactionState::GROWING);
        } 
        if(txn->get_state() != TransactionState::GROWING){
            throw TransactionAbortException (txn->get_txn_id(), AbortReason::LOCK_ON_SHRINKING);
        }

        Lock_data_id l_id(oid, p_id, Lock_data_type::PARTITION);
        Lock_data_id parent_table_l_id(oid, Lock_data_type::TABLE);

        // for benchmark debug
        // if(lock_mode == LockMode::SHARED || lock_mode == LockMode::INTENTION_SHARED){
        //     //检查父节点是否上IS锁
        //     if(txn->get_table_IS_lock_set()->count(parent_table_l_id)==0){
        //         throw TransactionAbortException(txn->get_txn_id(), AbortReason::TABLE_LOCK_NOT_PRESENT);
        //     }
        // } 
        // else if(lock_mode == LockMode::EXLUCSIVE || lock_mode == LockMode::INTENTION_EXCLUSIVE || lock_mode == LockMode::S_IX){
        //     //检查父节点是否上IX锁
        //     if(txn->get_table_IX_lock_set()->count(parent_table_l_id)==0){
        //         throw TransactionAbortException(txn->get_txn_id(), AbortReason::TABLE_LOCK_NOT_PRESENT);
        //     }
        // }
        
        //通过mutex申请全局锁表
        std::unique_lock<std::mutex> Latch(latch_);
        LockRequestQueue* request_queue = &lock_map_[l_id];
        std::unique_lock<std::mutex> queue_lock(request_queue->latch_);
        Latch.unlock();

        auto target_lock_mode = lock_mode;
        if(checkSameTxnLockRequest(txn, request_queue, target_lock_mode, queue_lock)==true){
            if(txn->get_state() == TransactionState::ABORTED)
                return false;
            if(txn->get_lock_set(Lock_data_type::PARTITION,target_lock_mode)->count(l_id)==0){
                txn->add_lock_set(target_lock_mode, l_id);
            }
            return true;
        }

        //检查是否可以上锁, 否则阻塞, 使用条件变量cv来实现
        LockRequest* lock_request = new LockRequest(txn->get_txn_id(), lock_mode); 
        request_queue->request_queue_.emplace_back(lock_request); 
        
        request_queue->cv_.wait(queue_lock, [&request_queue, &lock_request, &txn]{
                        //TODO deadlock
                        if(Lock_manager::enable_no_wait_==false)
                            return Lock_manager::checkQueueCompatible(request_queue, lock_request) || 
                                txn->get_state()==TransactionState::ABORTED;
                        if(Lock_manager::checkQueueCompatible(request_queue,lock_request)==true)
                            return true;
                        else{
                            txn->set_transaction_state(TransactionState::ABORTED);
                            return true;
                        }
                    });
        if(txn->get_state()==TransactionState::ABORTED) 
            throw TransactionAbortException (txn->get_txn_id(), AbortReason::DEAD_LOCK_PREVENT_NO_WAIT) ;
            
        lock_request->granted_ = true;
        txn->add_lock_set(lock_mode, l_id); 

        return true;
    }
    catch(TransactionAbortException &e)
    {
        txn->set_transaction_state(TransactionState::ABORTED);
        std::cerr << e.GetInfo() << '\n';
        return false;
    }
}

auto Lock_manager::LockRow(Transaction *txn, LockMode lock_mode, const table_oid_t &oid, const partition_id_t &p_id, const row_id_t &row_id) -> bool {

    try{
        if(txn->get_state() == TransactionState::DEFAULT){
            txn->set_transaction_state(TransactionState::GROWING);
        } 

        if(txn->get_state() != TransactionState::GROWING){
            // std :: cout << static_cast<int>(txn->get_state()) << "*********" << std::endl;
            throw TransactionAbortException (txn->get_txn_id(), AbortReason::LOCK_ON_SHRINKING);
        }
        if(lock_mode != LockMode::SHARED && lock_mode != LockMode::EXLUCSIVE){
            throw TransactionAbortException(txn->get_txn_id(), AbortReason::ATTEMPTED_INTENTION_LOCK_ON_ROW);
        }
        
        Lock_data_id l_id(oid, p_id, row_id, Lock_data_type::ROW);
        Lock_data_id parent_partition_l_id(oid, p_id, Lock_data_type::PARTITION);

        // 暂时注释，for benchmark
        // if(lock_mode == LockMode::SHARED || lock_mode == LockMode::INTENTION_SHARED){
        //     //检查父节点是否上IS锁
        //     if(txn->get_partition_IS_lock_set()->count(parent_partition_l_id)==0){
        //         throw TransactionAbortException(txn->get_txn_id(), AbortReason::PARTITION_LOCK_NOT_PRESENT);
        //     }
        // } 
        // else if(lock_mode == LockMode::EXLUCSIVE || lock_mode == LockMode::INTENTION_EXCLUSIVE || lock_mode == LockMode::S_IX){
        //     //检查父节点是否上IX锁
        //     if(txn->get_partition_IX_lock_set()->count(parent_partition_l_id)==0){
        //         throw TransactionAbortException(txn->get_txn_id(), AbortReason::PARTITION_LOCK_NOT_PRESENT);
        //     }
        // }

        //通过mutex申请全局锁表
        std::unique_lock<std::mutex> Latch(latch_);
        LockRequestQueue* request_queue = &lock_map_[l_id];
        std::unique_lock<std::mutex> queue_lock(request_queue->latch_);
        Latch.unlock();

        auto target_lock_mode = lock_mode;
        if(checkSameTxnLockRequest(txn, request_queue, target_lock_mode, queue_lock)==true){
            if(txn->get_state() == TransactionState::ABORTED)
                return false;
            if(txn->get_lock_set(Lock_data_type::ROW,target_lock_mode)->count(l_id)==0){
                txn->add_lock_set(target_lock_mode, l_id);
            }
            return true;
        }

        LockRequest* lock_request = new LockRequest(txn->get_txn_id(), lock_mode); 
        request_queue->request_queue_.emplace_back(lock_request); 
        
        request_queue->cv_.wait(queue_lock, [&request_queue, &lock_request, &txn]{
                        //TODO deadlock
                        if(Lock_manager::enable_no_wait_==false)
                            return Lock_manager::checkQueueCompatible(request_queue, lock_request) || 
                                txn->get_state()==TransactionState::ABORTED;
                        if(Lock_manager::checkQueueCompatible(request_queue,lock_request)==true)
                            return true;
                        else{
                            txn->set_transaction_state(TransactionState::ABORTED);
                            return true;
                        }
                    });
        if(txn->get_state()==TransactionState::ABORTED) 
            throw TransactionAbortException (txn->get_txn_id(), AbortReason::DEAD_LOCK_PREVENT_NO_WAIT) ;

        lock_request->granted_ = true;
        txn->add_lock_set(lock_mode, l_id); 

        return true;
    }
    catch(TransactionAbortException &e)
    {
        txn->set_transaction_state(TransactionState::ABORTED);
        // std::cerr << e.GetInfo() << '\n';
        return false;
    }
}

auto Lock_manager::UnLockTable(Transaction *txn,  const table_oid_t &oid) -> bool {

    Lock_data_id l_id(oid, Lock_data_type::TABLE);
    
    Unlock(txn, l_id);

    //由于锁升级, 需要将所有lock_set中的记录都删除
    txn->get_lock_set(Lock_data_type::TABLE, LockMode::SHARED)->erase(l_id);
    txn->get_lock_set(Lock_data_type::TABLE, LockMode::EXLUCSIVE)->erase(l_id);
    txn->get_lock_set(Lock_data_type::TABLE, LockMode::INTENTION_EXCLUSIVE)->erase(l_id);
    txn->get_lock_set(Lock_data_type::TABLE, LockMode::INTENTION_SHARED)->erase(l_id);
    txn->get_lock_set(Lock_data_type::TABLE, LockMode::S_IX)->erase(l_id);

    return true;
}

auto Lock_manager::UnLockPartition(Transaction *txn, const table_oid_t &oid, const partition_id_t &p_id ) -> bool {

    Lock_data_id l_id(oid, p_id, Lock_data_type::PARTITION);

    Unlock(txn, l_id);

    txn->get_lock_set(Lock_data_type::PARTITION, LockMode::SHARED)->erase(l_id);
    txn->get_lock_set(Lock_data_type::PARTITION, LockMode::EXLUCSIVE)->erase(l_id);
    txn->get_lock_set(Lock_data_type::PARTITION, LockMode::INTENTION_EXCLUSIVE)->erase(l_id);
    txn->get_lock_set(Lock_data_type::PARTITION, LockMode::INTENTION_SHARED)->erase(l_id);
    txn->get_lock_set(Lock_data_type::PARTITION, LockMode::S_IX)->erase(l_id);

    return true;
}

auto Lock_manager::UnLockRow(Transaction *txn, const table_oid_t &oid, const partition_id_t &p_id, const row_id_t &row_id) -> bool {

    Lock_data_id l_id(oid, p_id, row_id, Lock_data_type::ROW);

    Unlock(txn, l_id);

    txn->get_lock_set(Lock_data_type::ROW, LockMode::SHARED)->erase(l_id);
    txn->get_lock_set(Lock_data_type::ROW, LockMode::EXLUCSIVE)->erase(l_id);

    return true;
}

auto Lock_manager::Unlock(Transaction *txn, const Lock_data_id &l_id) -> bool {

    std::unique_lock<std::mutex> Latch(latch_);
    LockRequestQueue* request_queue = &lock_map_[l_id];
    std::unique_lock<std::mutex> queue_lock(request_queue->latch_);
    Latch.unlock();
    
    if(txn->get_state() == TransactionState::GROWING){
        txn->set_transaction_state(TransactionState::SHRINKING);
    }

    auto iter = request_queue->request_queue_.begin();
    for( ; iter != request_queue->request_queue_.end(); iter++){
        if((*iter)->txn_id_ == txn->get_txn_id() && (*iter)->granted_ == true){
            break;
        }
    }
    try{
        if(iter == request_queue->request_queue_.end()){
            throw TransactionAbortException(txn->get_txn_id(), AbortReason::ATTEMPTED_UNLOCK_BUT_NO_LOCK_HELD);
        }
    }
    catch(TransactionAbortException& e){
        txn->set_transaction_state(TransactionState::ABORTED);
        // std::cerr << e.GetInfo() << '\n';
        return false;
    }

    request_queue->request_queue_.erase(iter);
    request_queue->cv_.notify_all();
    return true;
} 
