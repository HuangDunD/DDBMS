#include <sstream>

#include "SSTableId.h"

SSTableId::SSTableId(const std::string &dir, uint64_t no)
    : dir_(dir), no_(no) {}

SSTableId::SSTableId(const SSTableId &sst) : dir_(sst.dir_), no_(sst.no_) {}

std::string SSTableId::name() const {
    std::ostringstream oss;
    oss << dir_ << "/" << no_ << ".sst";
    return oss.str();
}