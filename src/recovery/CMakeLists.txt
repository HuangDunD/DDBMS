# library recovery
add_library(recovery STATIC ${RECOVERY_SRC})

target_include_directories(recovery PUBLIC ${RECOVERY_DIR} ${STORAGE_DIR} ${INCLUDE_DIR} ${TRANSACTION_DIR})

target_link_libraries(recovery KV_STORAGE_BETA)