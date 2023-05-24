#ifndef STORAGE_TABLE_CACHE_H
#define STORAGE_TABLE_CACHE_H

#include <fstream>
#include <list>
#include <unordered_map>

#include "SSTableId.h"

class TableCache {
public:
   TableCache() = default;
   ~TableCache();
   std::ifstream* open(SSTableId no);
   void close(SSTableId no);
private:
   std::list<std::pair<uint64_t, std::ifstream*>> lists_;
   std::unordered_map<uint64_t, std::list<std::pair<uint64_t, std::ifstream*>>::iterator> map_;
};

#endif