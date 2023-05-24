#ifndef STORAGE_BLOCK_CACHE_H
#define STORAGE_BLOCK_CACHE_H

#include <string>
#include <unordered_map>
#include <cstdint>
#include <list>

#include "Location.h"

class BlockCache {
 public:
    explicit BlockCache();
    std::string read(Location &location);
 private:
    uint64_t num_blocks_;
    std::list<std::pair<std::string, std::string>> lists_;
    std::unordered_map<std::string, std::list<std::pair<std::string, std::string>>::iterator> map_;
};

#endif