
#include <butil/logging.h>
#include <brpc/channel.h>
#include <butil/time.h>
#include <thread>
#include "meta_service.pb.h"
// #include "meta_server.h"

#define server "[::1]:8001"

int main(){

    brpc::Channel channel;

    brpc::ChannelOptions options;
    options.timeout_ms = 100;
    options.max_retry = 3;

    if (channel.Init(server, &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        return -1;
    }

    meta_service::MetaService_Stub stub(&channel);

    // Send a request and wait for the response every 1 second.
    int log_id = 0;
    brpc::Controller cntl;

    std::thread test_oracle([&]{
        int log_id = 0;
        brpc::Controller cntl_oracle;
        

        meta_service::getTimeStampRequest request;
        meta_service::getTimeStampResponse response;
        while (!brpc::IsAskedToQuit())
        {
            cntl_oracle.Reset();

            cntl_oracle.set_log_id(log_id++);
            stub.GetTimeStamp(&cntl_oracle, &request, &response, NULL);
            if (!cntl_oracle.Failed()) {
                LOG(INFO) << "Received response from " << cntl.remote_side()
                    << " to " << cntl.local_side()
                    << ": TIMESTAMP" << response.timestamp() 
                    << " latency=" << cntl.latency_us() << "us";
            } else {
                LOG(WARNING) << cntl.ErrorText();
            }
            usleep(20 * 2000L);
        }
        
    });

    while (!brpc::IsAskedToQuit()) {
        cntl.Reset();

        meta_service::PartionkeyNameRequest request;
        meta_service::PartionkeyNameResponse response;

        request.set_db_name("test_db");
        request.set_tab_name("test_table");

        cntl.set_log_id(log_id ++);  

        // Because `done'(last parameter) is NULL, this function waits until
        // the response comes back or error occurs(including timedout).
        stub.GetPartitionKey(&cntl, &request, &response, NULL);
        if (!cntl.Failed()) {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                << " to " << cntl.local_side()
                << ": " << response.partition_key_name() 
                << cntl.response_attachment() << ")"
                << " latency=" << cntl.latency_us() << "us";
        } else {
            LOG(WARNING) << cntl.ErrorText();
        }
        usleep(2000 * 1000L);

        // brpc::Controller cntl;
        cntl.Reset();
        meta_service::PartitionLocationRequest request2;
        meta_service::PartitionLocationResponse response2;
        request2.set_db_name("test_db");
        request2.set_tab_name("test_table");
        request2.set_partition_key_name(response.partition_key_name());
        request2.mutable_string_partition_range()->set_min_range("100");
        request2.mutable_string_partition_range()->set_max_range("600");

        cntl.set_log_id(log_id ++);  
        stub.GetPartitionLocation(&cntl, &request2, &response2, NULL);

        if (!cntl.Failed()) {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                << " to " << cntl.local_side()
                << ": " ;
            auto res = response2.pid_partition_location();
            for(auto x : res){
                LOG(INFO) << "partition id: " << x.first << 
                    " ip address: "<< x.second.ip_addr() 
                    << " port: "<< x.second.port();
            }
            LOG(INFO) << " latency=" << cntl.latency_us() << "us";
        } else {
            LOG(WARNING) << cntl.ErrorText();
        }
        usleep(2000 * 1000L);

        cntl.Reset();
        meta_service::getTimeStampRequest request3;
        meta_service::getTimeStampResponse response3;
        cntl.set_log_id(log_id ++);  
        stub.GetTimeStamp(&cntl, &request3, &response3, NULL);

        if (!cntl.Failed()) {
            LOG(INFO) << "Received response from " << cntl.remote_side()
                << " to " << cntl.local_side()
                << ": " ;
            auto res = response3.timestamp();
            LOG(INFO) << res;
            LOG(INFO) << " latency=" << cntl.latency_us() << "us";
        } else {
            LOG(WARNING) << cntl.ErrorText();
        }
        usleep(2000 * 1000L);

    }

    LOG(INFO) << "MetaClient is going to quit";
    return 0;

}