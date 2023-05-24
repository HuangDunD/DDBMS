#ifndef STORAGE_LOCATION_H
#define STORAGE_LOCATION_H

#include <cstdint>

class SSTable;

struct Location {
    const SSTable *sst_;
    uint64_t pos_;
    uint64_t offset_;
    uint64_t len_;
    Location() = default;
    Location(const SSTable *sst, uint64_t pos, uint64_t offset, uint64_t len);
};

#endif