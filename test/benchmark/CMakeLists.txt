include_directories(./)

add_executable(benchmark_workload
    benchmark_workload.cpp 
    benchmark_config.cpp
    ../../meta_server/build/meta_service.pb.cc 
    ${EXECUTION_SRC}
)

add_executable(benchmark_test
    benchmark_test.cpp
    benchmark_txn.cpp
    benchmark_config.cpp
    ${EXECUTION_SRC}
    ../../meta_server/build/meta_service.pb.cc 
)

add_executable(data_test
    data_test.cpp
    benchmark_config.cpp
    ${EXECUTION_SRC}
    ../../meta_server/build/meta_service.pb.cc 
)

add_executable(participant
    participants.cpp
    benchmark_config.cpp
    ${EXECUTION_SRC}
    ../../meta_server/build/meta_service.pb.cc 
)

target_include_directories(benchmark_workload PRIVATE ${EXECUTION_DIR})
target_include_directories(benchmark_test PRIVATE ${EXECUTION_DIR})
target_include_directories(data_test PRIVATE ${EXECUTION_DIR})
target_include_directories(participant PRIVATE ${EXECUTION_DIR})

target_link_libraries(benchmark_workload transaction recovery config ${BRPC_LIB} ${DYNAMIC_LIB} gmock_main gtest)
target_link_libraries(benchmark_test transaction recovery config ${BRPC_LIB} ${DYNAMIC_LIB} gmock_main gtest)
target_link_libraries(data_test transaction recovery config ${BRPC_LIB} ${DYNAMIC_LIB} gmock_main gtest)
target_link_libraries(participant transaction recovery config ${BRPC_LIB} ${DYNAMIC_LIB} gmock_main gtest)