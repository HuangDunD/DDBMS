#include <fstream>
#include <filesystem>
#include <cassert>

#include "TableBuilder.h"
#include "snappy.h"

TableBuilder::TableBuilder(std::ofstream *file) 
                    : file_(file), offset_(0), size_(0), num_entries_(0), datablock_(), indexblock_(){

}

uint64_t TableBuilder::numEntries() const {
    return num_entries_;
}

// void TableBuilder::create_sstable(const SkipList *memtable) {
//     SkipList::Iterator iter(memtable);
//     iter.SeekToFirst();
//     while(iter.Valid()) {
//         add(iter.key(), iter.value());
//         iter.Next();
//     }
// }

void TableBuilder::add(const std::string &key, const std::string &value) {
    assert(file_->is_open());

    last_key_ = key;
    datablock_.add(key, value);
    num_entries_++;
    if(datablock_.estimated_size() >= Option::BLOCK_SPACE) {
        flush();
    }
}

void TableBuilder::flush() {
    assert(file_->is_open());
    if(datablock_.empty()) {
        return ;
    }
    writeBlock(&datablock_);
}

void TableBuilder::writeBlock(BlockBuilder *block) {
    assert(file_->is_open());
    // if compressed
    std::string block_content = block->finish();
    std::string compressed;
    if(Option::BLOCK_COMPRESSED) {
        snappy::Compress(block_content.data(), block_content.size(), &compressed);
    }else {
        compressed = block_content;
    }
    file_->write(compressed.data(), compressed.size());


    size_ = compressed.size();
    std::string s;
    s.append((char*)&offset_, sizeof(uint64_t));
    s.append((char*)&size_, sizeof(uint64_t));
    offset_ += size_;
    if(block != &indexblock_) {
        // if not index block, then insert handle to indexblock
        indexblock_.add(last_key_, s);
    }else {
        // if index block, insert handle finally
        file_->write(s.data(), s.size());
    }
    block->reset();
}

void TableBuilder::finish() {
    assert(file_->is_open());
    // write least data first
    flush();
    // write index block
    writeBlock(&indexblock_);
}

bool TableBuilder::create_sstable(const SkipList &memtable, const SSTableId & table_id) {
    std::ofstream ofs(table_id.name(), std::ios::binary);
    TableBuilder table_builder(&ofs);
    auto iter = SkipList::Iterator(&memtable);
    iter.SeekToFirst();
    while(iter.Valid()) {
        table_builder.add(iter.key(), iter.value());
        iter.Next();
    }
    table_builder.finish();
    ofs.close();
    return true;
}