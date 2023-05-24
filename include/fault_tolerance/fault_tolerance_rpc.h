#include <butil/logging.h> 
#include <brpc/server.h>
#include <gflags/gflags.h>

#include "fault_tolerance_service.pb.h"
#include "coordinator.h"

namespace fault_tolerance_service{
class CoorServiceImpl : public FaultToleranceService{
public:
    explicit CoorServiceImpl(Coordinator *coor) {coor_ = coor;};
    virtual ~CoorServiceImpl() {};

    virtual void getCoordinatorStatus(::google::protobuf::RpcController* controller,
                       const ::fault_tolerance_service::CoordinatorStatusRequest* request,
                       ::fault_tolerance_service::CoordinatorStatusResponse* response,
                       ::google::protobuf::Closure* done){

            brpc::ClosureGuard done_guard(done);

            response->set_activate(true);
            return;
    }
    
   virtual void getParticipantStatus(::google::protobuf::RpcController* controller,
                       const ::fault_tolerance_service::ParticipantStatusRequest* request,
                       ::fault_tolerance_service::ParticipantStatusResponse* response,
                       ::google::protobuf::Closure* done){

            brpc::ClosureGuard done_guard(done);
            
            response->set_activate(true);
            return;
    }

    virtual void NewCoordinator(::google::protobuf::RpcController* controller,
                       const ::fault_tolerance_service::NewCoordinatorRequest* request,
                       ::fault_tolerance_service::NewCoordinatorResponse* response,
                       ::google::protobuf::Closure* done){
            
            brpc::ClosureGuard done_guard(done);
            std::vector<IP_Port> ips;
            for(int i=0; i<request->ips_size(); i++){
                ips.push_back(IP_Port{request->ips()[i].ip(), request->ips()[i].port()});
            }
            coor_->Insert_txn(request->txn_id(), ips);
    }

private:
    Coordinator *coor_;
};
}
