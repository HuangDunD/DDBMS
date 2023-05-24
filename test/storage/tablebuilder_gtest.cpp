#include <string>
#include <fstream>
#include <iostream>

#include "gtest/gtest.h"
#include "TableBuilder.h"
#include "SSTable.h"

TEST(TableBuilder_TEST, emtpy_test) {
    std::ofstream ofs("table_empty_test", std::ios::binary);

    TableBuilder tablebuilder(&ofs);

    EXPECT_EQ(0, tablebuilder.numEntries());
    ofs.close();
}

TEST(TableBuilder_TEST, simple_test) {
    std::ofstream ofs("table_simple_test", std::ios::binary);

    TableBuilder tablebuilder(&ofs);
    const int N = 1024;

    for(int i = 0; i < N; i++) {
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        tablebuilder.add(key, value);
        EXPECT_EQ(i + 1, tablebuilder.numEntries());
    }
    tablebuilder.finish();
    ofs.close();

    // 
    // std::cout << "Complete build table, start read test \n";
    // read

    std::ifstream ifs("table_simple_test", std::ios::binary);
    if(ifs.is_open()) {
        SSTable sstable(&ifs, nullptr);
        for(int i = 0; i < N; i++) {
            std::string key(i + 1, 'a');
            std::string false_key(i + 1, 'c');
            std::string value(i + 1, 'b');

            EXPECT_EQ(std::make_pair(true, value), sstable.get(key));
            EXPECT_EQ(std::make_pair(false, std::string("")), sstable.get(false_key));
        }
    }
}

TEST(TableBuilder_TEST, large_test) {
    const std::string file_path = "table_large_test";
    std::ofstream ofs(file_path, std::ios::binary);

    TableBuilder tablebuilder(&ofs);
    const int N = 1024 * 4;

    for(int i = 0; i < N; i++) {
        std::string key(i + 1, 'a');
        std::string value(i + 1, 'b');
        tablebuilder.add(key, value);
        EXPECT_EQ(i + 1, tablebuilder.numEntries());
    }
    tablebuilder.finish();
    ofs.close();

    // 
    // std::cout << "Complete build table, start read test \n";
    // read

    std::ifstream ifs(file_path, std::ios::binary);
    if(ifs.is_open()) {
        SSTable sstable(&ifs, nullptr);
        for(int i = 0; i < N; i++) {
            std::string key(i + 1, 'a');
            std::string false_key(i + 1, 'c');
            std::string value(i + 1, 'b');

            EXPECT_EQ(std::make_pair(true, value), sstable.get(key));
            EXPECT_EQ(std::make_pair(false, std::string("")), sstable.get(false_key));
        }
    }
}


int main(int argc, char **argv) {
  printf("Running main() from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();   
}