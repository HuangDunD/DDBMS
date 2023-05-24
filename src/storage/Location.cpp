#include "Location.h"

Location::Location(const SSTable *sst, uint64_t pos, uint64_t offset, uint64_t len)
    : sst_(sst), pos_(pos), offset_(offset), len_(len) {}