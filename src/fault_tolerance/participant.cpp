#include "participant.h"
#include <brpc/channel.h>
#include <vector>
#include <future>

Participant::Participant(TransactionManager *transaction_maneger)
    :transaction_maneger_(transaction_maneger) {
        // Todo 参与者定期检查已准备事务的协调者状态
};