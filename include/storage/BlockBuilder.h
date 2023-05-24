#pragma once

#include <string>
#include <vector>

class BlockBuilder {
public:
    explicit BlockBuilder();

    BlockBuilder(const BlockBuilder &) = delete;
    BlockBuilder& operator=(const BlockBuilder &) = delete;

    // 插入键值对
    void add(const std::string &key, const std::string &value);

    // 完成写入，插入restart数组
    std::string finish();
    
    // reset the content
    void reset();

    size_t estimated_size() const;

    bool empty() const;

private:
    std::string buffer_;
    std::string last_key_;
    std::vector<uint64_t> restarts_;
    uint64_t counter_;
    bool finished_;
};