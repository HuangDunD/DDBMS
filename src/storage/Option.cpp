#include "Option.h"

const bool Option::BLOCK_COMPRESSED = true;

const uint64_t Option::BLOCK_SPACE = (uint64_t) 4 * 1024;
const uint64_t Option::SSTABLE_SPACE = (uint64_t)2 * 1024 * 1024;
const uint64_t Option::TABLE_CACHE_SIZE = 10;
const uint64_t Option::BLOCK_CACHE_SIZE = 100;
const uint64_t Option::RESTART_INTERVAL = 20;

const std::string Option::NAME_Z = "level_0";