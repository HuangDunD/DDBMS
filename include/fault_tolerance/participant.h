#pragma once

#include "transaction_manager.h"
#include "coordinator.h"
#include "fault_tolerance_rpc.h"
#include "meta_server_rpc.h"

class Participant
{
private:
    TransactionManager *transaction_maneger_;
public:
    explicit Participant(TransactionManager *transaction_maneger);
    ~Participant(){};
}; 

