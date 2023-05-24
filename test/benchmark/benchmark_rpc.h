#include <butil/logging.h> 
#include <brpc/server.h>
#include <gflags/gflags.h>

#include "benchmark_operator.pb.h"
#include "transaction_manager.h"

namespace benchmark_service {
class BenchmarkServiceImpl : public BenchmarkService{
public:
    explicit BenchmarkServiceImpl(TransactionManager *transaction_manager):transaction_manager_(transaction_manager) {};
    virtual void StartTxn(::google::protobuf::RpcController* controller,
                       const ::benchmark_service::StartTxnRequest* request,
                       ::benchmark_service::StartTxnResponse* response,
                       ::google::protobuf::Closure* done){

                brpc::ClosureGuard done_guard(done);
                
                uint64_t txn_id = request->txn_id();
                std::shared_lock<std::shared_mutex> l(transaction_manager_->txn_map_mutex);
                Transaction *txn =nullptr;
                int cnt = transaction_manager_->txn_map.count(txn_id);
                l.unlock();
                if(cnt){
                    txn = transaction_manager_->txn_map[txn_id];
                }else{
                    transaction_manager_->Begin(txn, txn_id);
                }
                response->set_ok(true);
                return;
    }

    virtual void SendOperator(::google::protobuf::RpcController* controller,
                       const ::benchmark_service::OperatorRequest* request,
                       ::benchmark_service::OperatorResponse* response,
                       ::google::protobuf::Closure* done){

                brpc::ClosureGuard done_guard(done);
                uint64_t txn_id = request->txn_id();
                
                Transaction* txn = transaction_manager_->getTransaction(txn_id);
                
                // std::shared_lock<std::shared_mutex> l(transaction_manager_->txn_map_mutex);
                // int cnt = transaction_manager_->txn_map.count(txn_id);
                // l.unlock();
                // if(cnt){
                //     txn = transaction_manager_->txn_map[txn_id];
                // }else{
                //     transaction_manager_->Begin(txn, txn_id);
                // }
                
                // transaction_manager_->getLockManager()->LockTable(txn, LockMode::S_IX, 0);
                // transaction_manager_->getLockManager()->LockPartition(txn, LockMode::S_IX, 0, 0);

                if(request->op_type() == OperatorRequest_OP_TYPE::OperatorRequest_OP_TYPE_Get){
                    std::string key = request->key();
                    int row_id = std::stoi(key.substr(3, key.length()-3));
                    // std::cout << "txn_id: " << txn_id << "row_id: "<< row_id << std::endl; 
                    // transaction_manager_->getLockManager()->LockTable(txn, LockMode::INTENTION_SHARED, 0);
                    // transaction_manager_->getLockManager()->LockPartition(txn, LockMode::INTENTION_SHARED, 0, 0);
                    if(transaction_manager_->getLockManager()->LockRow(txn, LockMode::SHARED, 0, 0, row_id) == false){
                        response->set_ok(false);
                        return;
                    }
                    std::string value = transaction_manager_->getKVstore()->get(key).second;
                    // std::string value = transaction_manager_->getKVstore()->get(row_id);
                }
                else if(request->op_type() == OperatorRequest_OP_TYPE::OperatorRequest_OP_TYPE_Put){
                    std::string key = request->key();
                    std::string value = request->value();
                    int row_id = std::stoi(key.substr(3, key.length()-3));
                    // std::cout << "txn_id: " << txn_id << "row_id: "<< row_id << std::endl; 
                    // transaction_manager_->getLockManager()->LockTable(txn, LockMode::INTENTION_EXCLUSIVE, 0);
                    // transaction_manager_->getLockManager()->LockPartition(txn, LockMode::INTENTION_EXCLUSIVE, 0, 0);
                    if(transaction_manager_->getLockManager()->LockRow(txn, LockMode::EXLUCSIVE, 0, 0, row_id)== false){
                        response->set_ok(false);
                        return;
                    }
                    transaction_manager_->getKVstore()->put(key, value, txn);
                }
                else if(request->op_type() == OperatorRequest_OP_TYPE::OperatorRequest_OP_TYPE_Del){
                    std::string key = request->key();
                    int row_id = std::stoi(key.substr(3, key.length()-3));
                    // std::cout << "txn_id: " << txn_id << "row_id: "<< row_id << std::endl; 
                    // transaction_manager_->getLockManager()->LockTable(txn, LockMode::INTENTION_EXCLUSIVE, 0);
                    // transaction_manager_->getLockManager()->LockPartition(txn, LockMode::INTENTION_EXCLUSIVE, 0, 0);
                    if(transaction_manager_->getLockManager()->LockRow(txn, LockMode::EXLUCSIVE, 0, 0, row_id)== false){
                        response->set_ok(false);
                        return;
                    }
                    transaction_manager_->getKVstore()->del(key, txn);
                }

                response->set_ok(true);
                return;
    }
private:
    TransactionManager *transaction_manager_;
};
}

