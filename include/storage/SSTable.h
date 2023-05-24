#ifndef STORAGE_SSTABLE_H
#define STORAGE_SSTABLE_H

#include <fstream>
#include <memory>

#include "BlockCache.h"
#include "Block.h"
#include "Iterator.h"

// TODO: Iterator and BlockCache
class SSTable {
public:

  explicit SSTable(std::ifstream *ifs, BlockCache *block_cache);

  // 禁用复制构造函数
  SSTable(const SSTable &) = delete;
  SSTable *operator=(const SSTable &) = delete;

  ~SSTable();

  std::pair<bool, std::string> get(const std::string &key) const;

  // 
  static std::unique_ptr<Block> loadBlock(std::ifstream *ifs, uint64_t block_offset, uint64_t block_size);
private:
  // class Iter;
  std::ifstream *ifs_;
  // std::string last_key_;
  
  std::unique_ptr<Block> index_block_;
  std::unique_ptr<Iterator> index_iter_;

  BlockCache *block_cache_;

};

// #include <string>
// #include <cstdint>
// #include <vector>

// #include "SkipList.h"
// #include "Location.h"
// #include "SSTableId.h"
// #include "TableCache.h"
// #include "Option.h"
// #include "BlockCache.h"

// class SSTable{
//   public:
//     explicit SSTable(const SSTableId &id, TableCache *table_cache, BlockCache *block_cache);
//     explicit SSTable(const SkipList &memtable, const SSTableId &id, TableCache *table_cache, BlockCache *block_cache);
//     std::string loadBlock(uint64_t pos) const;
//     uint64_t no() const;
//     std::pair<bool, std::string> search(uint64_t key);
//   private:
//     SSTableId id_;
//     TableCache *table_cache_;
//     BlockCache *block_cache_;
//     uint64_t num_entries_;
//     uint64_t num_blocks_;
//     std::vector<uint64_t> keys_;
//     std::vector<uint64_t> offsets_;
//     std::vector<uint64_t> oris_;
//     std::vector<uint64_t> cmps_;
//     // std::vector<std::string> values;
    
//     void save(const std::string &blockSeg) const;
//     uint64_t indexSpace() const;
//     Location locate(uint64_t pos) const;
// };



#endif