#ifndef STORAGE_ENTRY_H
#define STORAGE_ENTRY_H

#include <cstdint>
#include <string>

struct Entry {
   std::string key_;
   std::string value_;

   Entry(const std::string &key, const std::string &value);
   ~Entry() = default;
};

#endif