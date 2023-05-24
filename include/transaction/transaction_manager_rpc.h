#include <butil/logging.h> 
#include <brpc/server.h>
#include <gflags/gflags.h>

#include "transaction_manager.pb.h"
#include "transaction_manager.h"

namespace transaction_manager {
class TransactionManagerImpl : public TransactionManagerService{
public:
    TransactionManagerImpl(TransactionManager *transaction_manager):transaction_manager_(transaction_manager) {};
    TransactionManagerImpl() {};
    virtual ~TransactionManagerImpl() {};

    // TODO 接收远端协调者发送的事务回滚的brpc服务端方法
    virtual void AbortTransaction(::google::protobuf::RpcController* controller,
                       const ::transaction_manager::AbortRequest* request,
                       ::transaction_manager::AbortResponse* response,
                       ::google::protobuf::Closure* done){

                brpc::ClosureGuard done_guard(done);

                return;
        }

    // TODO 接收远端协调者发送的事务准备的brpc服务端方法
    virtual void PrepareTransaction(::google::protobuf::RpcController* controller,
                       const ::transaction_manager::PrepareRequest* request,
                       ::transaction_manager::PrepareResponse* response,
                       ::google::protobuf::Closure* done){

                brpc::ClosureGuard done_guard(done);

                return;
        }
    
    // TODO 接收远端协调者发送的事务提交的brpc服务端方法
    virtual void CommitTransaction(::google::protobuf::RpcController* controller,
                       const ::transaction_manager::CommitRequest* request,
                       ::transaction_manager::CommitResponse* response,
                       ::google::protobuf::Closure* done){

            brpc::ClosureGuard done_guard(done);

            return;
       }

private:
    TransactionManager *transaction_manager_;
 };
} 