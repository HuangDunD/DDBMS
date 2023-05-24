#include <string>

#include "Option.h"
#include "KVStore_beta.h"

KVStore_beta::KVStore_beta(const std::string &dir) : KVStoreAPI(dir), diskstorage_(dir){
    
}

KVStore_beta::~KVStore_beta() {
	flush();
}

// put(key, value)
void KVStore_beta::put(const std::string & key, const std::string &value){
    // TODO
    std::unique_lock<std::mutex> l(mutex_);
    memtable_.put(key, value);
    if(memtable_.space() > Option::SSTABLE_SPACE){
        diskstorage_.add(memtable_);
        memtable_.clear();
    }
}


// value = get(key)
std::pair<bool, std::string> KVStore_beta::get(const std::string & key){
    std::pair<bool, std::string> result;
    std::unique_lock<std::mutex> l(mutex_);
    if(memtable_.contains(key)){
        result = memtable_.get(key);
    }else {
        l.unlock();
        result = diskstorage_.search(key);
    }
    if(result.first && result.second != "") {
        return result;
    }
    return std::make_pair(false, "");
}

// del(key)
bool KVStore_beta::del(const std::string & key){
    std::unique_lock<std::mutex> l(mutex_);
    if(memtable_.contains(key)){
        memtable_.del(key);
        return true;
    }
    l.unlock();
    auto result = diskstorage_.search(key);
    if(result.first && result.second != ""){
        l.lock();
        memtable_.put(key, "");
        return true;
    }else{
        return false;
    }
}

// void KVStore_beta::put(uint64_t key, const std::string &value, Transaction *txn) {
//     memtable_.put(key, value);
//     if(memtable_.space() > Option::SSTABLE_SPACE){
//         diskstorage_.add(memtable_);
//         memtable_.clear();
//     }
// }

// std::string KVStore_beta::get(uint64_t key, Transaction *txn) {

//     if(memtable_.contains(key)){
//         return memtable_.get(key);
//     }
//     auto result = diskstorage_.search(key);
//     return result.second;
// }

// // TODO design delete
// bool KVStore_beta::del(uint64_t key, Transaction *txn) {
//     if(memtable_.contains(key)){
//         memtable_.del(key);
//         return true;
//     }
//     auto result = diskstorage_.search(key);
//     if(result.first){
//         memtable_.put(key, "");
//         return true;
//     }else{
//         return false;
//     }
// }

void KVStore_beta::reset() {
    memtable_.clear();
    // memtable_.clear();
    // diskstorage_.clear();
}

void KVStore_beta::flush() {
    if(!memtable_.empty()){
        diskstorage_.add(memtable_);
    }
}