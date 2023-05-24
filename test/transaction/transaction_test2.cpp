#include "gtest/gtest.h"
#include "transaction_manager.h"
#include "transaction_manager_rpc.h"
#include "meta_server_rpc.h"
#include "execution_rpc.h"

#include <filesystem>
#include <brpc/channel.h>

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
        std::string dir = "./data2";
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

        std::thread rpc_thread_trans([&]{
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
        rpc_thread_trans.detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
};

TEST_F(TransactionTest, TransactionTest1){
    while(kv_->get("key2").second!="value2"){
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    std::cout << "key2, value2" << std::endl;

    while(kv_->get("key4").second!="value4"){
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    std::cout << "key4, value4" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

int main(int argc, char **argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}