#ifndef STORAGE_LEVEL_ZERO
#define STORAGE_LEVEL_ZERO

#include <string>
#include <cstdint>

#include "SSTableId.h"
#include "SkipList.h"
#include "TableCache.h"
#include "BlockCache.h"
#include "SSTable.h"

// TODO table_cache
class LevelZero {
public:
  // explicit 
  explicit LevelZero(const std::string &dir, TableCache* table_cache, BlockCache *block_cache);
  // 禁用复制构造函数和复制赋值函数
  LevelZero(const LevelZero &) = delete;
  LevelZero &operator=(const LevelZero &) = delete;

  // 用memtable创建新的SSTable
  void add(const SkipList &memtable, uint64_t no);
  // 查找key
  std::pair<bool, std::string> search(std::string key);

  inline uint64_t size() { return size_; }
private:
  std::string dir_;
  std::vector<SSTableId> ssts_;
  uint64_t size_;
  uint64_t num_entries_;

  TableCache *table_cache_;
  BlockCache *block_cache_;

  void save_meta() const;
};

#endif