#include <cassert>

#include "BlockBuilder.h"
#include "Option.h"

BlockBuilder::BlockBuilder() : restarts_(), counter_(0), finished_(false) {
    assert(Option::RESTART_INTERVAL >= 1);
    restarts_.push_back(0);
}

void BlockBuilder::add(const std::string &key, const std::string &value) {
    assert(!finished_);
    assert(counter_ <= Option::RESTART_INTERVAL);
    assert(empty() || key > last_key_);
    // write restarts_
    if(counter_ ==  Option::RESTART_INTERVAL){
        restarts_.push_back(buffer_.size());
        counter_ = 0;
    }

    uint64_t key_size = key.size();
    uint64_t value_size = value.size();

    buffer_.append((char*)&key_size, sizeof(uint64_t));
    buffer_.append(key);
    buffer_.append((char*)&value_size, sizeof(uint64_t));
    buffer_.append(value);

    last_key_ = key;

    counter_++;
}

bool BlockBuilder::empty() const {
    return buffer_.empty();
}

std::string BlockBuilder::finish() {
    uint64_t restarts_postion = buffer_.size();
    uint64_t num = restarts_.size();
    // append size
    buffer_.append((char*)&num, sizeof(uint64_t));
    // append restarts_
    for(auto i : restarts_){
        buffer_.append((char *)&i, sizeof(uint64_t));
    }
    // append postion
    buffer_.append((char *)&restarts_postion, sizeof(uint64_t));
    // finished
    finished_ = true;

    return buffer_;
}

void BlockBuilder::reset() {
    buffer_.clear();
    restarts_.clear();
    counter_ = 0;
    last_key_.clear();
    finished_ = false;

    restarts_.push_back(0);
}

size_t BlockBuilder::estimated_size() const {
    return (buffer_.size() + 
            restarts_.size() * sizeof(uint64_t) + 
            sizeof(uint64_t) * 2);
}