#pragma once

#include "transaction_manager.h"

class Coordinator
{
private:
    std::shared_ptr<std::unordered_map<txn_id_t, Transaction*>> coor_txn_map;
    std::shared_mutex map_mutex; 

    TransactionManager *transaction_maneger_;

public:
    explicit Coordinator(TransactionManager* transaction_maneger)
        :transaction_maneger_(transaction_maneger){
            coor_txn_map = std::make_shared<std::unordered_map<txn_id_t, Transaction*>>();
    };
    ~Coordinator(){};
    
    std::shared_ptr<std::unordered_map<txn_id_t, Transaction*>> get_coor_txn_map() {return coor_txn_map;}
    TransactionManager * getTransactionManeger(){ return transaction_maneger_; }

    //作为新协调者接收新事务, 如果有一个参与者收到了prepare请求之后断绝了和协调者的联系, 则选取新协调者
    void Insert_txn(txn_id_t txn_id, std::vector<IP_Port> ips);

};


