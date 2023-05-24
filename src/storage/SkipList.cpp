#include <iostream>
#include <cassert>

#include "SkipList.h"
#include "Option.h"

SkipList::SkipList() : dist_(0, 1)
{
    init();
}

SkipList::~SkipList()
{
    clear();
    delete head_;
    delete tail_;
}

// clear all nodes in skiplist, include head_ and tail_
void SkipList::clear()
{
    for (Node *node = head_; node != nullptr;)
    {
        Node *next = node->nexts_[0];
        delete node;
        node = next;
    }
    init();
}

// get the value of the key, if not found, return empty string
std::pair<bool, std::string> SkipList::get(const std::string &key) const
{
    Node *node = findGreatorOrEqual(key, nullptr);
    if (node != tail_ && node->key_ == key)
    {
        return std::make_pair(true, node->value_);
    }
    else
    {
        return std::make_pair(false, "");
    }
    // return node != head_ && node->key_ == key ? node->value_ : "";
}

// insert a new (key, value) pair into skiplist, if the key is already in
// the skiplist, then update the value, if not, then create a new node using
void SkipList::put(const std::string &key, const std::string &value)
{
    // find key
    Node *prevs[max_height_];
    Node *x = findGreatorOrEqual(key, prevs);
    // if equal, update the value
    if (x != tail_ && x->key_ == key)
    {
        num_bytes_ = num_bytes_ - x->value_.size() + value.size();
        x->value_ = value;
        return;
    }
    size_t height = random_height();
    if (height > getMaxHeight())
    {
        for (size_t i = getMaxHeight(); i < height; i++)
        {
            prevs[i] = head_;
        }
        enlargeHeight(height);
    }
    // update the pointers
    Node *node = new Node(key, value, height);
    for (size_t i = 0; i < height; i++)
    {
        node->nexts_[i] = prevs[i]->nexts_[i];
        prevs[i]->nexts_[i] = node;
    }
    num_entries_++;
    num_bytes_ = num_bytes_ + key.size() + value.size();
    return;
}

// delete (key, value) pair
bool SkipList::del(const std::string &key)
{
    // find the node
    Node *prevs[max_height_];
    Node *node = findGreatorOrEqual(key, prevs);
    if (node == tail_ || node->key_ != key)
    {
        return false;
    }
    size_t height = node->height_;
    for (size_t i = 0; i < height; i++)
    {
        prevs[i]->nexts_[i] = node->nexts_[i];
    }
    num_entries_--;
    num_bytes_ = num_bytes_ - node->key_.size() - node->value_.size();
    delete node;
    return true;
}

bool SkipList::contains(const std::string &key) const
{
    Node *node = findGreatorOrEqual(key, nullptr);
    return node != tail_ && node->key_ == key;
}



// uint64_t SkipList::space() const {
//     return (num_entries_ * 2 + num_bytes_ / Option::BLOCK_SPACE * 2 + 6) * sizeof(uint64_t) + num_bytes_;
// }

// SkipList::Iterator SkipList::newIterator() const {
//     return Iterator(this);
// }

void SkipList::init()
{
    head_ = new Node("", "", 1);
    tail_ = new Node("", "", 1);
    head_->nexts_[0] = tail_;
    tail_->nexts_[0] = nullptr;
    num_bytes_ = 0;
    num_entries_ = 0;
}

size_t SkipList::random_height()
{
    size_t height = 1;
    while (dist_(engine_))
        ++height;
    return height > max_height_ ? max_height_ : height;
}

size_t SkipList::getMaxHeight() const
{
    return head_->height_;
}

void SkipList::enlargeHeight(size_t height)
{
    size_t oldHeight = head_->height_;
    head_->height_ = height;
    tail_->height_ = height;
    Node **oldHeadNexts = head_->nexts_;
    Node **oldTailNexts = tail_->nexts_;
    head_->nexts_ = new Node *[height];
    tail_->nexts_ = new Node *[height];
    for (size_t i = 0; i < height; ++i)
        tail_->nexts_[i] = nullptr;
    for (size_t i = 0; i < oldHeight; ++i)
    {
        head_->nexts_[i] = oldHeadNexts[i];
    }
    for (size_t i = oldHeight; i < height; ++i)
    {
        head_->nexts_[i] = tail_;
    }
    delete[] oldHeadNexts;
    delete[] oldTailNexts;
}

// TODO findGreatorOrEqual
// return the first node that greater or equal to the given key
// if no such value, return tail_
SkipList::Node *SkipList::findGreatorOrEqual(const std::string &key, Node **prevs) const
{
    Node *x = head_;
    int height = getMaxHeight() - 1;
    while (true)
    {
        Node *next = x->nexts_[height];
        if (next != tail_ && next->key_ < key)
        {
            // search in this level
            x = next;
        }
        else
        {
            if (prevs != nullptr)
            {
                prevs[height] = x;
            }
            if (height == 0)
            {
                return next;
            }
            else
            {
                height--;
            }
        }
    }
}

// TODO find less Than key
SkipList::Node* SkipList::findLessThan(const std::string &key) const {
    Node *node = head_;
    size_t level = getMaxHeight() - 1;
    while(true) {
        Node *next = node->nexts_[level];
        if(next != tail_ && next->key_ < key){
            node = next;
        }else {
            if(level == 0){
                return node;
            }else{
                level--;
            }
        }
    }
}

//
SkipList::Node *SkipList::findLast() const {
    Node *node = head_;
    size_t level = getMaxHeight() - 1;
    while(true) {
        Node *next = node->nexts_[level];
        if(next != tail_) {
            node = next;
        }else {
            if(level == 0) {
                return node;
            }else{
                level--;
            }
        }
    }
}

// Node
SkipList::Node::Node(const std::string &key, const std::string &value, size_t height)
    : key_(key), value_(value), height_(height)
{
    nexts_ = new Node *[height];
}

SkipList::Node::~Node()
{
    delete[] nexts_;
}

//Iterator
SkipList::Iterator::Iterator(const SkipList *skiplist) : skiplist_(skiplist), node_(nullptr){

}

// 
bool SkipList::Iterator::Valid() const {
    return node_ != nullptr;
}

// 
void SkipList::Iterator::SeekToFirst() {
    node_ = skiplist_->head_->nexts_[0];
    if(node_ == skiplist_->tail_){
        node_ = nullptr;
    }
}

// 
void SkipList::Iterator::SeekToLast() {
    node_ = skiplist_->findLast(); 
    if(node_ == skiplist_->head_){
        node_ = nullptr;
    }   
}

//
void SkipList::Iterator::Seek(const std::string &key) {
    node_ = skiplist_->findGreatorOrEqual(key, nullptr);
    if(node_ == skiplist_->tail_) {
        node_ = nullptr;
    }
}

//
void SkipList::Iterator::Next() {
    assert(Valid());
    node_ = node_->nexts_[0];
    if(node_ == skiplist_->tail_){
        node_ = nullptr;
    }
}

//
void SkipList::Iterator::Prev() {
    assert(Valid());
    node_ = skiplist_->findLessThan(node_->key_);
    if(node_ == skiplist_->head_){
        node_ = nullptr;
    }
}

//
std::string SkipList::Iterator::key() const {
    assert(Valid());
    return node_->key_;
}

//
std::string SkipList::Iterator::value() const {
    assert(Valid());
    return node_->value_;
}