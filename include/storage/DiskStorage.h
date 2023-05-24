#ifndef STORAGE_DISK_STORAGE
#define STORAGE_DISK_STORAGE

#include <string>
#include <cstdint>

#include "SkipList.h"
#include "LevelZero.h" 
#include "BlockCache.h"
#include "TableCache.h"

class DiskStorage{
public:
   explicit DiskStorage(const std::string &dir);

   DiskStorage(const DiskStorage &) = delete;
   DiskStorage &operator=(const DiskStorage &) = delete;

   void add(const SkipList &memtable);

   std::pair<bool, std::string> search(std::string key);
private:
   std::string dir_;
   uint64_t no_;
   // BlockCache block_cache_;
   // TableCache table_cache_;
   LevelZero level0_;
   
   void read_meta();
   void save_meta() const;
};

#endif