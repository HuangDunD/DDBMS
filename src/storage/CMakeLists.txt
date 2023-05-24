option(STORAGE_BETA "STORAGE_BETA" ON)
option(STORAGE_ORIGIN "KVStore" OFF) 

set(STORAGE_DIR ${PROJECT_SOURCE_DIR}/include/storage/)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/storage STORAGE_SRC)

IF(STORAGE_BETA)
    list(REMOVE_ITEM STORAGE_SRC "${PROJECT_SOURCE_DIR}/src/storage/KVStore.cpp")
    add_library(KV_STORAGE_BETA STATIC ${STORAGE_SRC})
    target_include_directories(KV_STORAGE_BETA PUBLIC ${STORAGE_DIR})
    target_link_libraries(KV_STORAGE_BETA snappy)
ENDIF()

# library KV_STORAGE

IF(STORAGE_ORIGIN)
    add_library(KV_STORAGE STATIC ${STORAGE_SRC})
    target_include_directories(KV_STORAGE PUBLIC ${STORAGE_DIR})
    target_link_libraries(KV_STORAGE snappy)
ENDIF()