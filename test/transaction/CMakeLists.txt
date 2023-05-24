add_executable(transaction_test
    transaction_test.cpp 
    ${EXECUTION_SRC}
    ../../meta_server/build/meta_service.pb.cc 
)

add_executable(transaction_test2
    transaction_test2.cpp 
    ${EXECUTION_SRC}
    ../../meta_server/build/meta_service.pb.cc 
)

add_executable( LockManager_test
    Lock_manager_test.cpp
    ../../src/transaction/Lock_manager.cpp
    
)
target_include_directories(LockManager_test PRIVATE ${TRANSACTION_DIR} ${INCLUDE_DIR})
target_link_libraries(LockManager_test gmock_main gtest)

target_include_directories(transaction_test PRIVATE ${EXECUTION_DIR})
target_include_directories(transaction_test2 PRIVATE ${EXECUTION_DIR})
target_link_libraries(transaction_test transaction recovery config ${BRPC_LIB} ${DYNAMIC_LIB} gmock_main gtest)
target_link_libraries(transaction_test2 transaction recovery config ${BRPC_LIB} ${DYNAMIC_LIB} gmock_main gtest)