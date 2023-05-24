#include <butil/logging.h>
#include <brpc/server.h>
#include "meta_service.pb.h" 
#include "meta_server.h"

namespace meta_service{
class MetaServiceImpl : public MetaService{
public:
    MetaServiceImpl(MetaServer *meta_server):meta_server_(meta_server) {};
    MetaServiceImpl() {};
    virtual ~MetaServiceImpl() {};

    virtual void GetPartitionKey( google::protobuf::RpcController* cntl_base,
                      const PartionkeyNameRequest* request,
                      PartionkeyNameResponse* response,
                      google::protobuf::Closure* done);

    virtual void GetPartitionLocation(google::protobuf::RpcController* cntl_base,
                      const PartitionLocationRequest* request,
                      PartitionLocationResponse* response,
                      google::protobuf::Closure* done);
    
    virtual void CreateDataBase(::google::protobuf::RpcController* controller,
                       const ::meta_service::CreateDatabaseRequest* request,
                       ::meta_service::CreateDatabaseResponse* response,
                       ::google::protobuf::Closure* done){
                        
            brpc::ClosureGuard done_guard(done);

            if(meta_server_->get_db_map().count(request->db_name())==1){
                //db已经存在
                response->set_success(false);
            }else{
                meta_server_->mutable_db_map()[request->db_name()] = new DbMetaServer(request->db_name());
                response->set_success(true);
            }
    };

    //创建非分区表函数, 由meta_server分配table_oid, 并将Partition Type设置成None partition
    virtual void CreateTable(::google::protobuf::RpcController* controller,
                       const ::meta_service::CreateTableRequest* request,
                       ::meta_service::CreateTableResponse* response,
                       ::google::protobuf::Closure* done);
    
    //创建分区表
    virtual void CreatePartitionTable(::google::protobuf::RpcController* controller,
                       const ::meta_service::CreatePartitonTableRequest* request,
                       ::meta_service::CreatePartitonTableResponse* response,
                       ::google::protobuf::Closure* done);

    //节点注册
    virtual void NodeRegister(::google::protobuf::RpcController* controller,
                       const ::meta_service::RegisterRequest* request,
                       ::meta_service::RegisterResponse* response,
                       ::google::protobuf::Closure* done){

            brpc::ClosureGuard done_guard(done);

            brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);

            // 考虑到每台机器可能运行多个server进程, 这里ip_node_map的key值为ip:port
            // std::string ip_port = butil::endpoint2str(cntl->remote_side()).c_str();
            
            std::string server_name = request->server_name();

            if(meta_server_->mutable_ip_node_map().count(server_name)>0){
                // meta_server_->mutable_ip_node_map()[butil::ip2str(cntl->remote_side().ip).c_str()]->port = cntl->remote_side().port;
                // meta_server_->mutable_ip_node_map()[butil::ip2str(cntl->remote_side().ip).c_str()]->activate = true;
                meta_server_->mutable_ip_node_map()[server_name]->activate = true;
            }
            else{
                // meta_server_->mutable_ip_node_map()[butil::ip2str(cntl->remote_side().ip).c_str()] = new Node(
                //     butil::ip2str(cntl->remote_side().ip).c_str(), cntl->remote_side().port, true);

                meta_server_->mutable_ip_node_map()[server_name] = new Node(
                    butil::ip2str(cntl->remote_side().ip).c_str(), request->server_listen_port(), true);
            }
            response->set_register_ok(true);
    }

    //分片换主
    virtual void UpdateLeader(::google::protobuf::RpcController* controller,
                    const ::meta_service::UpdateLeaderRequest* request,
                    ::meta_service::UpdateLeaderResponse* response,
                    ::google::protobuf::Closure* done){
            brpc::ClosureGuard done_guard(done);
            brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
            std::string db_name = request->db_name();
            std::string tab_name = request->tab_name();
            int32_t par_id = request->par_id();
            std::string ip_addr = butil::ip2str(cntl->remote_side().ip).c_str();
            meta_server_->UpdatePartitionLeader(db_name, tab_name, par_id, ip_addr);
    }

    virtual void GetTimeStamp(::google::protobuf::RpcController* controller,
                       const ::meta_service::getTimeStampRequest* request,
                       ::meta_service::getTimeStampResponse* response,
                       ::google::protobuf::Closure* done){
            //注意 brpc::ClosureGuard done_guard(done) 这一行, 极易忘记!!!
            brpc::ClosureGuard done_guard(done);
            
            auto now = meta_server_->get_oracle().getTimeStamp();
            response->set_timestamp(now);
    }
    // 获取表的列名信息
    virtual void GetTableInfo(::google::protobuf::RpcController* cntl_base,
                       const ::meta_service::GetTableInfoRequest* request,
                       ::meta_service::GetTableInfoResponse* response,
                       ::google::protobuf::Closure* done){
            //注意 brpc::ClosureGuard done_guard(done) 这一行, 极易忘记!!!
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
        
        LOG(INFO) << "Received request[log_id=" << cntl->log_id() 
            << "] from " << cntl->remote_side() 
            << " to " << cntl->local_side()
            << ": db_name: " << request->db_name() << "table name: " << request->tab_name();
        
        auto Col = meta_server_->GetColInfor(request->db_name(),request->tab_name());
        int size = Col.column_name.size();
        for(int i = 0; i < size; i++){
            response->add_col_name(Col.column_name[i]);
            response->add_col_type(int(Col.column_type[i]));
        }
    }
    // 确定性获取新协调者, 同一个错误的协调者一定返回相同的新协调者
    virtual void GetNewCoordinator(::google::protobuf::RpcController* controller,
                       const ::meta_service::getNewCoorRequest* request,
                       ::meta_service::getNowCoorResponse* response,
                       ::google::protobuf::Closure* done){
        brpc::ClosureGuard done_guard(done);

        std::shared_lock<std::shared_mutex> l(meta_server_->get_mutex());
        for(auto iter= meta_server_->mutable_ip_node_map().begin(); 
                iter != meta_server_->mutable_ip_node_map().end(); ++iter){
            if(iter->second->ip_addr == request->fault_ip() && iter->second->port == request->port()){
                // find the fault node, return the next node
                ++iter;
                if(iter == meta_server_->mutable_ip_node_map().end()){
                    iter = meta_server_->mutable_ip_node_map().begin();
                }
                response->set_new_coor_ip(iter->second->ip_addr.c_str());
                response->set_port(iter->second->port);
                return;
            }
        }
    }
private: 
    MetaServer *meta_server_;
};
}

