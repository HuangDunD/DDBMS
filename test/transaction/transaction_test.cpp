#include "gtest/gtest.h"
#include "transaction_manager.h"
#include "transaction_manager_rpc.h"
#include "meta_server_rpc.h"
#include "execution_rpc.h"

#include <brpc/channel.h>
#include <filesystem>

DEFINE_string(SERVER_NAME, "", "Server NAME");

class TransactionTest : public ::testing::Test {
   public:
    std::unique_ptr<Lock_manager> lock_manager_;
    std::unique_ptr<LogStorage> log_storage_;
    std::unique_ptr<LogManager> log_manager_;
    std::unique_ptr<KVStore> kv_;
    std::unique_ptr<TransactionManager> transaction_manager_;

   public:
    void SetUp() override {
        ::testing::Test::SetUp();
        std::string dir = "./data";
        lock_manager_ = std::make_unique<Lock_manager>(true);
        log_storage_ = std::make_unique<LogStorage>("test_db");
        log_manager_ = std::make_unique<LogManager>(log_storage_.get());
        if(std::filesystem::exists(dir)) {
            std::filesystem::remove_all(dir);
            std::filesystem::remove(dir);
        }
        kv_ = std::make_unique<KVStore>(dir, log_manager_.get());
        transaction_manager_ = std::make_unique<TransactionManager>(lock_manager_.get(), kv_.get(), log_manager_.get(), 
            ConcurrencyMode::TWO_PHASE_LOCKING);

        brpc::Channel channel;
        brpc::ChannelOptions options;
        options.timeout_ms = 100;
        options.max_retry = 3;
        
        if (channel.Init(FLAGS_META_SERVER_ADDR.c_str(), &options) != 0) {
            LOG(ERROR) << "Fail to initialize channel";
        }
        meta_service::MetaService_Stub stub(&channel);
        
        meta_service::RegisterRequest request;
        meta_service::RegisterResponse response;
        while (!brpc::IsAskedToQuit() && !response.register_ok()){
            brpc::Controller cntl;
            request.set_server_name(FLAGS_SERVER_NAME);
            request.set_server_listen_port(FLAGS_SERVER_LISTEN_PORT);
            request.set_sayhello(true);
            stub.NodeRegister(&cntl, &request, &response, NULL);
            if(response.register_ok())
                std::cout << "Register success! ip: " << cntl.local_side() << std::endl;
        }

        std::thread rpc_thread([&]{
            //启动事务brpc server
            brpc::Server server;

            transaction_manager::TransactionManagerImpl trans_manager_impl(transaction_manager_.get());
            if (server.AddService(&trans_manager_impl, 
                                    brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
                LOG(ERROR) << "Fail to add service";
            }

            distributed_plan_service::RemotePlanNodeImpl remote_plannode_impl(transaction_manager_.get());
            if (server.AddService(&remote_plannode_impl, 
                                    brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
                LOG(ERROR) << "Fail to add service";
            }

            butil::EndPoint point;
            if (!FLAGS_SERVER_LISTEN_ADDR.empty()) {
                if (butil::str2endpoint(FLAGS_SERVER_LISTEN_ADDR.c_str(),FLAGS_SERVER_LISTEN_PORT, &point) < 0) {
                    LOG(ERROR) << "Invalid listen address:" << FLAGS_SERVER_LISTEN_ADDR;
                }
            } else {
                point = butil::EndPoint(butil::IP_ANY, FLAGS_SERVER_LISTEN_PORT);
            }

            brpc::ServerOptions options;
            
            if (server.Start(point,&options) != 0) {
                LOG(ERROR) << "Fail to start Server";
            }

            server.RunUntilAskedToQuit();
        });
        rpc_thread.detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
};

TEST_F(TransactionTest, TransactionTest1){
    // 写入(key1,value1)
    // 定义一个空事务指针
    Transaction* txn1 = nullptr;
    transaction_manager_->Begin(txn1);

    // SQL解析之后到metaserver查询相关表在两个server中
    txn1->set_is_distributed(true);
    txn1->get_distributed_node_set()->push_back(IP_Port{"127.0.0.1", 8002});
    txn1->get_distributed_node_set()->push_back(IP_Port{"127.0.0.1", 8003}); 

    brpc::Channel channel;
    brpc::ChannelOptions options;
    options.timeout_ms = 0x7fffffff;
    options.max_retry = 3;

    if (channel.Init("127.0.0.1:8003", &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
    }
    distributed_plan_service::RemotePlanNode_Stub stub_exec(&channel);

    distributed_plan_service::RemotePlan request_plan;
    distributed_plan_service::ValuePlan response_plan;
    
    request_plan.set_txn_id(txn1->get_txn_id());
    distributed_plan_service::InsertPlan *insert_plan = new distributed_plan_service::InsertPlan();
    insert_plan->set_db_name("test_db");
    insert_plan->set_tab_id(1);
    insert_plan->set_par_id(2);
    distributed_plan_service::ValuePlan *v = new distributed_plan_service::ValuePlan();
    v->add_value("key2");
    v->add_value("value2");
    v->add_child();
    // distributed_plan_service::ChildPlan vc;
    // vc.set_allocated_value_plan(&v);
    insert_plan->add_child();
    insert_plan->mutable_child(0)->set_allocated_value_plan(v);
    request_plan.mutable_child()->set_allocated_insert_plan(insert_plan);

    brpc::Controller cntl;
    stub_exec.SendRemotePlan(&cntl, &request_plan, &response_plan, NULL);
    if (!cntl.Failed()) {
        std::cout << "success send plan. " << std::endl;
    } else {
        LOG(WARNING) << cntl.ErrorText();
    }

    cntl.Reset();

    lock_manager_->LockTable(txn1, LockMode::INTENTION_EXCLUSIVE, 1);
    lock_manager_->LockPartition(txn1, LockMode::EXLUCSIVE, 1 , 1);
    kv_->put("key1", "value1", txn1);
        
    transaction_manager_->Commit(txn1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(txn1->get_state(), TransactionState::COMMITTED);

    ASSERT_EQ(kv_->get("key1").second, "value1");

    // try to abort a transaction
    Transaction* txn2 = nullptr;
    transaction_manager_->Begin(txn2);

    // SQL解析之后到metaserver查询相关表在两个server中
    txn2->set_is_distributed(true);
    txn2->get_distributed_node_set()->push_back(IP_Port{"127.0.0.1", 8002});
    txn2->get_distributed_node_set()->push_back(IP_Port{"127.0.0.1", 8003}); 

    request_plan.set_txn_id(txn1->get_txn_id());
    distributed_plan_service::InsertPlan *insert_plan2 = new distributed_plan_service::InsertPlan();
    insert_plan2->set_db_name("test_db");
    insert_plan2->set_tab_id(1);
    insert_plan2->set_par_id(2);
    distributed_plan_service::ValuePlan *v2 = new distributed_plan_service::ValuePlan();
    v2->add_value("key4");
    v2->add_value("value4");
    v2->add_child();
    // distributed_plan_service::ChildPlan vc;
    // vc.set_allocated_value_plan(&v);
    insert_plan2->add_child();
    insert_plan2->mutable_child(0)->set_allocated_value_plan(v2);
    request_plan.mutable_child()->set_allocated_insert_plan(insert_plan2);

    cntl.Reset();
    stub_exec.SendRemotePlan(&cntl, &request_plan, &response_plan, NULL);
    if (!cntl.Failed()) {
        std::cout << "success send plan. " << std::endl;
    } else {
        LOG(WARNING) << cntl.ErrorText();
    }
    cntl.Reset();

    lock_manager_->LockTable(txn2, LockMode::INTENTION_EXCLUSIVE, 1);
    lock_manager_->LockPartition(txn2, LockMode::EXLUCSIVE, 1 , 1);
    kv_->put("key3", "value3", txn2);

    ASSERT_EQ(txn2->get_state(), TransactionState::GROWING);
    ASSERT_EQ(kv_->get("key3").second, "value3");

    transaction_manager_->Abort(txn2);
    ASSERT_EQ(txn2->get_state(), TransactionState::ABORTED);
    ASSERT_EQ(kv_->get("key3").second, "");
    
}

int main(int argc, char **argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    testing::InitGoogleTest(&argc, argv);
    
    return RUN_ALL_TESTS();
}