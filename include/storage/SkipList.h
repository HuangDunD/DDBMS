#ifndef STORAGE_SKIPLIST_H
#define STORAGE_SKIPLIST_H

#include <string>
#include <cstddef>
#include <random>
#include <utility>

#include "Entry.h"

class SkipList
{
public:
    class Iterator;
    explicit SkipList();
    ~SkipList();

    // insert (key, value) pair into the skiplist
    // if there already exists a key that equals to the given key,
    // then update the value
    void put(const std::string &key, const std::string &value);

    // get value for the given key, if the key value pair doesn't exist
    // return <false, "">
    std::pair<bool, std::string> get(const std::string &key) const;

    // delete the (key, value) pair in skiplist, if the (key, value) pair
    // doesn't exist, return false
    bool del(const std::string &key);

    // return true if the key in skiplist
    bool contains(const std::string &key) const;

    // Iterator newIterator() const;

    // return how many (key, value) pairs in skiplist
    inline size_t size() const { return num_entries_; }

    // return the space of skiplist
    inline size_t space() const { return num_bytes_; }

    // return true, if the skiplist is empty
    inline bool empty() const { return num_entries_ ==  0; }

    // clear the skiplist
    // Require: before clear the skiplist, store the data in SSTable
    void clear();

    // return the amount of skiplist's space
    // size_t space() const;
private:
    struct Node;

    const size_t max_height_ = 12;

    Node *head_, *tail_;

    size_t num_entries_;

    size_t num_bytes_;

    std::default_random_engine engine_;
    std::uniform_int_distribution<int> dist_;

    // create head_ and tail_ node in skiplist
    void init();

    // enlarge the height of head_ and tail_ in skiplist
    void enlargeHeight(size_t height);

    // return a random height that is not greater than max_height_
    size_t random_height();

    // return the current max height in skiplist
    size_t getMaxHeight() const;

    // find the first node which is greater or euqal to the given key
    // and if prevs is not nullptr, node's prevs nodes will be store in prevs
    Node *findGreatorOrEqual(const std::string &key, Node **prevs) const;
    
    //
    Node *findLessThan(const std::string &key) const;

    //
    Node *findLast() const;
};

struct SkipList::Node
{
    std::string key_;
    std::string value_;
    Node **nexts_;
    size_t height_;
    explicit Node(const std::string &key, const std::string &value, size_t height);
    Node() = delete;
    ~Node();
};

class SkipList::Iterator {
public:
    explicit Iterator(const SkipList *skiplist);

    // don't use copy constructor and assignment operator
    Iterator(const Iterator &) = delete;
    Iterator &operator=(const Iterator &) = delete;

    ~Iterator() = default;

    // Is iterator at a key/value pair. Before use, call this function
    bool Valid() const;

    // 
    void SeekToFirst();

    // 
    void SeekToLast();

    //
    void Seek(const std::string &key);
    
    //
    void Next();
    
    //
    void Prev();

    //
    std::string key() const;

    //
    std::string value() const;
private:
    const SkipList *skiplist_;
    Node *node_;
};

#endif