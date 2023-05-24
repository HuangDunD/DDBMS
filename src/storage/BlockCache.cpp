#include <sstream>

#include "BlockCache.h"
#include "SSTable.h"
#include "Option.h"

BlockCache::BlockCache() : num_blocks_(0) {}

std::string BlockCache::read(Location &location) {
    std::ostringstream oss;
    // oss << location.sst_->no_ << "-" << location.pos_;
    std::string blockId = oss.str();
    std::string block;

    if(map_.count(blockId)){
        block = map_[block]->second;
        lists_.erase(map_[block]);
        lists_.emplace_front(blockId, block);
        map_[block] = lists_.begin();
    }else{
        if(num_blocks_ == Option::BLOCK_CACHE_SIZE){
            map_.erase(lists_.back().first);
            num_blocks_--;
            lists_.pop_back();
        }
        // block = location.sst_->loadBlock(location.pos_);
        ++num_blocks_;
        lists_.emplace_front(blockId, block);
        map_[blockId] = lists_.begin();
    }
    return block.substr(location.offset_, location.len_);
}