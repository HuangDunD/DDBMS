#pragma once

#include <string>
#include <cstdint>

struct SSTableId{
   std::string dir_;
   uint64_t no_;  

   SSTableId(const std::string &dir, uint64_t no);

   SSTableId(const SSTableId & sst);

   ~SSTableId() =  default;
   
   std::string name() const;
};