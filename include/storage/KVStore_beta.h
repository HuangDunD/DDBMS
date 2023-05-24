# pragma once

#include <cstdint>
#include <string>
#include <mutex>

#include "KVStoreAPI.h"
#include "SkipList.h"
#include "DiskStorage.h"
 
class KVStore_beta : KVStoreAPI{
public:
   explicit KVStore_beta(const std::string &dir);
   
   KVStore_beta(const KVStore_beta &) = delete;
   KVStore_beta &operator=(const KVStore_beta &) = delete;

   ~KVStore_beta();

   // put(key, value)
   void put(const std::string & key, const std::string &value);
   // value = get(key)
   std::pair<bool, std::string> get(const std::string & key);
   // del(key)
   bool del(const std::string & key);
   
   // clear memtable and disk
   void reset();
   // flush memtable to disk 
   void flush();
protected:
   std::mutex mutex_;
   SkipList memtable_;
   DiskStorage diskstorage_;
};