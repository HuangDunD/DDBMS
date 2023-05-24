option(STORAGE_TEST OFF)
option(SKIPLIST_TEST "SKIPLIST_TEST" ON)
option(BLOCKBUILDER_TEST "block builder's test" ON)
option(TABLEBUILDER_TEST "table builder's test" ON)
option(LEVELZERO_TEST "level0's test" ON)
option(DISKSTORAGE_TEST "disk storage's test" ON)
option(KVSTORAGE_BETA_TEST "kvstorage beta's test" ON)

if(STORAGE_TEST)
    # storage test
    add_executable(correctness ./correctness.cpp)
    target_link_libraries(correctness KV_STORAGE) 
    # storage googletest
    add_executable(correct_gtest ./correct_gtest.cpp)
    target_link_libraries(correct_gtest KV_STORAGE gmock_main gtest)
endif()

# skiplist
if(SKIPLIST_TEST)
    message("storage test: build skiplist_test")
    add_executable(skiplist_gtest 
                        ${PROJECT_SOURCE_DIR}/test/storage/skiplist_gtest.cpp 
                        ${PROJECT_SOURCE_DIR}/src/storage/SkipList.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Entry.cpp)
    target_include_directories(skiplist_gtest PRIVATE ${STORAGE_DIR})
    target_link_libraries(skiplist_gtest gmock_main gtest)
endif()

# block builder test
if(BLOCKBUILDER_TEST) 
    message("storage test: build block builder test")
    add_executable(blockbuilder_gtest
                        ${PROJECT_SOURCE_DIR}/test/storage/blockbuilder_gtest.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/BlockBuilder.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Block.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Iterator.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Option.cpp)
    target_include_directories(blockbuilder_gtest PRIVATE ${STORAGE_DIR})
    target_link_libraries(blockbuilder_gtest gmock_main gtest)
endif()

# table builder test
if(TABLEBUILDER_TEST)
    message("storage test: table builder test")
    add_executable(tablebuilder_gtest
                        ${PROJECT_SOURCE_DIR}/test/storage/tablebuilder_gtest.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/TableBuilder.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/BlockBuilder.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Iterator.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Block.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SSTable.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SkipList.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SSTableId.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Option.cpp
                        )
    target_include_directories(tablebuilder_gtest PRIVATE ${STORAGE_DIR})
    target_link_libraries(tablebuilder_gtest gmock_main gtest snappy)
endif()


if(LEVELZERO_TEST)
    message("storage test: build level0's test")
    add_executable(level0_gtest
                        ${PROJECT_SOURCE_DIR}/test/storage/LevelZero_gtest.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/TableBuilder.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/BlockBuilder.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Iterator.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Block.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SSTable.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/LevelZero.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/TableCache.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SSTableId.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SkipList.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Option.cpp
                        )
    target_include_directories(level0_gtest PRIVATE ${STORAGE_DIR})
    target_link_libraries(level0_gtest gmock_main gtest snappy)
endif()

if(DISKSTORAGE_TEST) 
    message("storage test: build disk storage's test")
    add_executable(diskstorage_gtest
                        ${PROJECT_SOURCE_DIR}/test/storage/diskstorage_gtest.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/DiskStorage.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/TableBuilder.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/BlockBuilder.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Iterator.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Block.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SSTable.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/LevelZero.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/TableCache.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SSTableId.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SkipList.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Option.cpp
                        )
    target_include_directories(diskstorage_gtest PRIVATE ${STORAGE_DIR})
    target_link_libraries(diskstorage_gtest gmock_main gtest snappy)
endif()

if(KVSTORAGE_BETA_TEST) 
    message("storage test: kv storage beta's test")
    add_executable(kvstorage_beta_gtest
                        ${PROJECT_SOURCE_DIR}/test/storage/kvstorage_beta_gtest.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/KVStore_beta.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/DiskStorage.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/TableBuilder.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/BlockBuilder.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Iterator.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Block.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SSTable.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/LevelZero.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/TableCache.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SSTableId.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/SkipList.cpp
                        ${PROJECT_SOURCE_DIR}/src/storage/Option.cpp
                        )
    target_include_directories(kvstorage_beta_gtest PRIVATE ${STORAGE_DIR})
    target_link_libraries(kvstorage_beta_gtest gmock_main gtest snappy)
endif()

