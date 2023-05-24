add_executable(log_manager_test log_manager_test.cpp)

target_link_libraries(log_manager_test recovery config gmock_main gtest ${GFLAGS_LIBRARY})