#ifndef STORAGE_LRU_REPLACER_H
#define STORAGE_LRU_REPLACER_H

#include <mutex>
#include <list>
#include <map>

// TODO implement a LRU cache policy

template <typename T>
class LRUReplacer{
 public:
    explicit LRUReplacer(uint64_t num);
    ~LRUReplacer();
    bool victim(T* t_id);
    void pin(T* t_id);
    void unpin(T* t_id);
    uint64_t size();
 private:
    std::mutex lru_replacer_mutex_;
    std::list<T> lists_;
};

#endif