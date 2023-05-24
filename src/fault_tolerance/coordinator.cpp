#include "coordinator.h"

void Coordinator::Insert_txn(txn_id_t txn_id, std::vector<IP_Port> ips) {
    std::shared_lock<std::shared_mutex> l(map_mutex);
    if(coor_txn_map->count(txn_id) != 0){
        // 已经收到协调者宕机而换新协调者的请求
    }
    else{
        l.unlock();
        std::unique_lock<std::shared_mutex> l(map_mutex);
        Transaction* txn = new Transaction(txn_id);
        if(ips.size()>1){
            txn->set_is_distributed(true);
        }
        else{
            txn->set_is_distributed(false);    
        }
        for(auto x: ips){
            txn->get_distributed_node_set()->push_back(x);
        }
        coor_txn_map->emplace(txn_id, txn);
        // 开启一个后台线程作为协调者
        std::thread coordinator_prepare([&txn, this]{
            this->getTransactionManeger()->Commit(txn);
            std::unique_lock<std::shared_mutex> l(map_mutex);
            this->get_coor_txn_map()->erase(txn->get_txn_id());
        });
        coordinator_prepare.detach();
    }
    return;
}

