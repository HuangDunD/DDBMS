#pragma once

#include <string>

class Iterator{
public:
    Iterator();
    
    // don't use copy constructor and assignment operator
    Iterator(const Iterator&) = delete;
    Iterator& operator=(const Iterator&) = delete;

    virtual ~Iterator();

    // Is iterator at a key/value pair. Before use, call this function
    virtual bool Valid() const = 0;

    // 
    virtual void SeekToFirst() = 0;

    // 
    virtual void SeekToLast() = 0;

    //
    virtual void Seek(const std::string &key) = 0;
    
    //
    virtual void Next() = 0;
    
    //
    virtual void Prev() = 0;

    //
    virtual std::string Key() const = 0;

    //
    virtual std::string Value() const = 0;
    
private:

};