#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Iterator.h"
#include "Option.h"

class Block {
public:
    explicit Block(const std::string &block_content);

    // 禁用复制构造函数
    Block(const Block &) = delete;
    Block& operator=(const Block &) = delete;

    // get(key)
    std::pair<bool, std::string> get(const std::string &key) const;

    // new iterator
    std::unique_ptr<Iterator> NewIterator() const;

    // size of block
    // size_t size() const;

private:
    // 迭代器Iter
    class Iter;

    void extract(uint64_t position, uint64_t *size, std::string &s) const;

    // block内容
    const std::string block_content_;
    // restart offset
    uint64_t restart_offset_;
    // restart size
    size_t restart_size_;
    // restarts_数组
    std::vector<uint64_t> restarts_;
    // size
    // size_t size_;
};

class Block::Iter : public Iterator {
public:
    Iter(const Block *block);
    
    ~Iter();

    // Is iterator at a key/value pair. Before use, call this function
    bool Valid() const override;

    // 
    void SeekToFirst() override;

    // 
    void SeekToLast() override;

    //
    void Seek(const std::string &key) override;
    
    //
    void Next() override;
    
    //
    void Prev() override;

    //
    std::string Key() const;

    //
    std::string Value() const;
private:
    const Block *block_;

    uint64_t current_;
    std::string key_;
    std::string value_;

    void extract_keyvalue();
};