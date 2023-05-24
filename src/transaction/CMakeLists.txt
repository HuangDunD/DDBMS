# library transaction
add_library(transaction STATIC ${TRANSACTION_SRC})

target_include_directories(transaction PUBLIC ${TRANSACTION_DIR} ${INCLUDE_DIR} ${STORAGE_DIR} ${IMMEMORY_DIR} ${RECOVERY_DIR})

target_link_libraries(transaction KV_STORAGE_BETA proto ${BRPC_LIB} ${DYNAMIC_LIB})
