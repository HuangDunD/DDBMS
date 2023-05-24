#include <butil/logging.h>
#include <brpc/server.h>
#include "distributed_plan_service.pb.h"
#include "planner.h"
#include "transaction/transaction_manager.h"

#define listen_addr "[::0]:8002"
#define idle_timeout_s -1

/*
**这是接收远程执行计划的节点, 从远程发送来的执行计划首先应该从brpc中解析到
**单机可执行的执行计划数据结构之后正常执行
*/

namespace distributed_plan_service{
class RemotePlanNodeImpl : public RemotePlanNode{
public:
    RemotePlanNodeImpl(TransactionManager *transaction_manager) {transaction_manager_ = transaction_manager; };
    virtual ~RemotePlanNodeImpl() {};

    virtual void SendRemotePlan( google::protobuf::RpcController* cntl_base,
                      const RemotePlan* request,
                      ValuePlan* response,
                      google::protobuf::Closure* done);

    void ConvertIntoPlan(const ChildPlan* child_plan, std::shared_ptr<Operators> operators, Transaction* txn);

private:
    TransactionManager *transaction_manager_;
};

}