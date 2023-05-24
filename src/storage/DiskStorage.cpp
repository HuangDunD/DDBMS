#include <filesystem>
#include <fstream>

#include "DiskStorage.h"
#include "Option.h"

DiskStorage::DiskStorage(const std::string &dir) : dir_(dir), level0_(dir_ + "/" + Option::NAME_Z, nullptr, nullptr){
    if(!std::filesystem::exists(dir_)){
        std::filesystem::create_directory(dir_);
    }
    // if no meta file, create meta file
    read_meta();
}

void DiskStorage::add(const SkipList &memtable){
    level0_.add(memtable, no_);
    no_++;
    // if level0 overflows, call merger
    save_meta();
}

std::pair<bool, std::string> DiskStorage::search(std::string key) {
    return level0_.search(key);
}

void DiskStorage::read_meta() {
    if(std::filesystem::exists(std::filesystem::path(dir_ + "/meta"))){
        std::ifstream ifs(dir_ + "/meta", std::ios::binary);
        ifs.read((char*) &no_, sizeof(uint64_t));
        ifs.close();
    }else{
        no_ = 0;
        save_meta();
    }
}

void DiskStorage::save_meta() const {
    std::ofstream ofs(dir_ + "/meta", std::ios::binary);
    // byte stream
    ofs.write((char*)&no_, sizeof(uint64_t));
    ofs.close();
}