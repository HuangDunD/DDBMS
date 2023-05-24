#ifndef STORAGE_OPTION_H
#define STORAGE_OPTION_H

#include <string>
#include <cstdint>

namespace Option {
    extern const bool BLOCK_COMPRESSED;

    extern const std::string NAME_Z;

    extern const uint64_t RESTART_INTERVAL;
    extern const uint64_t BLOCK_SPACE;
    extern const uint64_t SSTABLE_SPACE;
    extern const uint64_t TABLE_CACHE_SIZE;
    extern const uint64_t BLOCK_CACHE_SIZE;
}

#endif