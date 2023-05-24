#include "execution_rpc.h"

namespace distributed_plan_service{
void RemotePlanNodeImpl::SendRemotePlan( google::protobuf::RpcController* cntl_base,
                      const RemotePlan* request,
                      ValuePlan* response,
                      google::protobuf::Closure* done){

        brpc::ClosureGuard done_guard(done);
        // brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);

        uint64_t txn_id = request->txn_id();
        std::shared_lock<std::shared_mutex> l(transaction_manager_->txn_map_mutex);
        Transaction *txn =nullptr;
        if(transaction_manager_->txn_map.count(txn_id) != 0){
            txn = transaction_manager_->txn_map[txn_id];
        }else{
            l.unlock();
            transaction_manager_->Begin(txn, txn_id);
        }

        // std::shared_ptr<Operators> operators = std::make_shared<Operators>();
        std::shared_ptr<Operators> operators = nullptr;
        ConvertIntoPlan(&request->child(), operators, txn);
        
}

void RemotePlanNodeImpl::ConvertIntoPlan(const ChildPlan* child_plan, std::shared_ptr<Operators> operators, Transaction *txn){
    auto cur_op = operators;
    while (child_plan->child_plan_case() != ChildPlan::ChildPlanCase::CHILD_PLAN_NOT_SET){
        switch ( child_plan->child_plan_case()){
            case ChildPlan::ChildPlanCase::kSeqScanPlan :{
                auto ptr = std::make_shared<op_tablescan>();
                ptr->db_name = child_plan->seq_scan_plan().db_name();
                ptr->tabs = child_plan->seq_scan_plan().tab_name();
                ptr->par_id = child_plan->seq_scan_plan().par_id();

                cur_op = ptr;
                cur_op = cur_op->next_node;
                child_plan = &child_plan->project_plan().child()[0];
                break;
            }
            case ChildPlan::ChildPlanCase::kFilterPlan :{

                break;
            }
            case ChildPlan::ChildPlanCase::kProjectPlan :{
                auto ptr = std::make_shared<op_projection>();
                std::string tab_name = child_plan->project_plan().tab_name();
                for(auto col : child_plan->project_plan().col_name()){
                    ptr->cols.push_back(std::make_shared<ast::Col>(tab_name,col));
                }
                
                cur_op = ptr;
                cur_op = cur_op->next_node;
                child_plan = &child_plan->project_plan().child()[0];
                break;
            }
            case ChildPlan::ChildPlanCase::kInsertPlan :{
                std::cout << "kinsertplan" << std::endl;
                // hcy temp code to test transaction
                std::string db_name = child_plan->insert_plan().db_name();
                int32_t tab_id = child_plan->insert_plan().tab_id();
                int32_t par_id = child_plan->insert_plan().par_id();

                std::string key = child_plan->insert_plan().child()[0].value_plan().value()[0];
                std::string value = child_plan->insert_plan().child()[0].value_plan().value()[1];

                auto lock_manager = transaction_manager_->getLockManager();
                lock_manager->LockTable(txn, LockMode::INTENTION_EXCLUSIVE ,tab_id);
                lock_manager->LockPartition(txn, LockMode::EXLUCSIVE ,tab_id, par_id);

                transaction_manager_->getKVstore()->put(key, value, txn);

                child_plan = &child_plan->insert_plan().child()[0].value_plan().child()[0];
                break;
            }
            case ChildPlan::ChildPlanCase::kDeletePlan :
                /* code */
                break;
            case ChildPlan::ChildPlanCase::kUpdatePlan :
                /* code */
                break;
            case ChildPlan::ChildPlanCase::kNestedloopJoinPlan : {
                auto ptr = std::make_shared<op_join>();
                for(int i=0; i<child_plan->nestedloop_join_plan().child_size(); i++){
                    std::shared_ptr<Operators> ptr_child;
                    ConvertIntoPlan(&child_plan->nestedloop_join_plan().child()[i], ptr_child, txn);
                    ptr->tables_get.push_back(ptr_child);
                }
                cur_op = ptr;
                return;
            }
            case ChildPlan::ChildPlanCase::kTableGetPan :
                /* code */
                break;
            case ChildPlan::ChildPlanCase::kValuePlan :
                /* code */
                break;
            default:
                break;
        }
    }
   
}
}