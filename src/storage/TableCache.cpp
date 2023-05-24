#include "TableCache.h"
#include "Option.h"


TableCache::~TableCache() {
    for(auto it : lists_){
        it.second->close();
        delete it.second;
    }
}

std::ifstream *TableCache::open(SSTableId id) {
    // in map, return ifstream directly, and update the position in list
    if(map_.count(id.no_)){
        lists_.push_front(*map_[id.no_]);
        lists_.erase(map_[id.no_]);
        map_[id.no_] = lists_.begin();
        return map_[id.no_]->second;
    }
    if(lists_.size() == Option::TABLE_CACHE_SIZE){
        // not in map and full
        map_.erase(lists_.back().first);
        lists_.back().second->close();
        delete lists_.back().second;
        lists_.pop_back();
    }
    std::ifstream *ifs = new std::ifstream(id.name(), std::ios::binary);
    lists_.emplace_front(id.no_, ifs);
    map_[id.no_] = lists_.begin();
    return ifs;   
}

void TableCache::close(SSTableId id) {
    if(!map_.count(id.no_)){
        return;
    }
    map_[id.no_]->second->close();
    lists_.erase(map_[id.no_]);
    delete map_[id.no_]->second;
    map_.erase(id.no_);
}