#include <cstring>
#include <cassert>

#include <iostream>

#include "Block.h"

Block::Block(const std::string &block_content) : block_content_(block_content){
    
    const char *block = block_content_.c_str();

    // 读取restart_offset
    std::memcpy(&restart_offset_, block + block_content_.size() - sizeof(uint64_t), sizeof(uint64_t));
    // 读取restart_size
    std::memcpy(&restart_size_, block + restart_offset_, sizeof(uint64_t));

    // 读取restarts_
    uint64_t start = restart_offset_ + sizeof(uint64_t);
    uint64_t end = block_content_.size() - sizeof(uint64_t);
    for(auto i = start; i < end; i += sizeof(uint64_t)){
        uint64_t buffer;
        std::memcpy(&buffer, block + i, sizeof(uint64_t));
        restarts_.push_back(buffer);
    }
}

// TODO: 暂时全局顺序查找，后续优化为先使用二分查找restart，然后顺序查找内部
std::pair<bool, std::string> Block::get(const std::string &key) const {
    const char *block = block_content_.c_str();

    uint64_t pos = 0;
    while(pos < restart_offset_) {
        uint64_t key_offset, value_offset;
        uint64_t key_size, value_size;
        // assignmemt
        key_offset = pos;
        std::memcpy(&key_size, block + key_offset, sizeof(uint64_t));
        value_offset = key_offset + sizeof(uint64_t) + key_size;
        std::memcpy(&value_size, block + value_offset, sizeof(uint64_t));
        // if s equals to the given key, return 
        std::string s = block_content_.substr(key_offset + sizeof(uint64_t), key_size);
        if(s == key) {

            // std::cout << "key offset = " << key_offset << std::endl;
            // std::cout << "key size = " << key_size << std::endl;
            // std::cout << "value offset = " << value_offset << std::endl;
            // std::cout << "value size = " << value_size << std::endl;

            return std::make_pair(true, block_content_.substr(value_offset + sizeof(uint64_t), value_size));
        }
        pos = value_offset + value_size + sizeof(uint64_t);
    }
    return std::make_pair(false, "");
    // for(auto restart : restarts_) {
    //     uint64_t size;
    //     std::memcpy(&size, block + restart, sizeof(uint64_t));
    //     std::string s = block_content_.substr(restart + sizeof(uint64_t), size);
    //     if()
    // }
}

void Block::extract(uint64_t position, uint64_t *size, std::string &s) const {
    // const char *block = block_content_.c_str();

    // std::memcpy(size, block + position, sizeof(uint64_t));
    // s = block_content_.substr(position + sizeof(uint64_t), *size);
}

std::unique_ptr<Iterator> Block::NewIterator() const {
    return std::make_unique<Iter>(this);
}

Block::Iter::Iter(const Block *block) : block_(block) {
    current_ = block_->restart_offset_;
    // SeekToFirst();
}

Block::Iter::~Iter() {
    
}

bool Block::Iter::Valid() const {
    return current_ < block_->restart_offset_;
}

void Block::Iter::SeekToFirst() {
    current_ = 0;
    if(Valid()) {
        extract_keyvalue();
    }
}

void Block::Iter::SeekToLast() {
    // 定位到最后一个restart点
    current_ = block_->restarts_.back();
    extract_keyvalue();
    // 然后顺序查找
    while(true) {
        // 如果下一个位置不合法，则退出
        uint64_t next = current_ + sizeof(uint64_t) * 2 + key_.size() + value_.size();
        if(next >= block_->restart_offset_) {
            return ;
        }
        // 如果合法，则调用next函数
        Next();
    }
}

// 找到第一个大于或者等于key的位置
void Block::Iter::Seek(const std::string &key) {
    // 首先查看是否存在这样的位置
    SeekToLast();
    if(key_ < key) {
        current_ = block_->restart_offset_;
        return ;
    }
    // 从第一个位置开始查找
    SeekToFirst();
    while(key_ < key) {
        Next();
        if(!Valid()){
            return ;
        }
    }
    // // 首先通过restart数组定位
    // for(auto pos : block_->restarts_) {
    //     current_ = pos;
    //     extract_keyvalue();
    //     // if given key > key_, then next, else turn back and break
    //     if(key > key_) {
    //         //继续

    //     }else if (key == key_) {
    //         return ;
    //     }else if (key < key_) {
    //         // 当前key_大于given key

    //     } 
    //     //
    // }
}

void Block::Iter::Next() {
    current_ += sizeof(uint64_t) * 2 + key_.size() + value_.size();
    if(Valid()) {
        extract_keyvalue();
    }
}

void Block::Iter::Prev() {
    return ;
}

void Block::Iter::extract_keyvalue() {
    assert(Valid());

    uint64_t key_offset, value_offset;
    uint64_t key_size, value_size;

    const char *block = block_->block_content_.c_str();
    key_offset = current_;
    // key size
    std::memcpy(&key_size, block + key_offset, sizeof(uint64_t));
    key_ = block_->block_content_.substr(key_offset + sizeof(uint64_t), key_size);
    // value
    value_offset = key_offset + sizeof(uint64_t) + key_size;
    std::memcpy(&value_size, block + value_offset, sizeof(uint64_t));
    value_ = block_->block_content_.substr(value_offset + sizeof(uint64_t), value_size);
}

std::string Block::Iter::Key() const {
    assert(Valid());
    return key_;
}

std::string Block::Iter::Value() const {
    assert(Valid());
    return value_;
}

