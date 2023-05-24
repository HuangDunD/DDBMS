#include <string>
#include <iostream>

#include "KVStore_beta.h"

int main() {
    // 初始化需要一个空文件夹或者不存在的文件夹
    KVStore_beta store("./data");
    // put(key, value)接口, key 和 value都是std::string类型
    store.put("Yang", "ShiMing");
    // get(key)接口，返回类型为std::pair<bool, std::string>，第一个参数表示该值是否存在，第二个参数是value
    auto result = store.get("Yang");
    std::cout << "result.first = " << result.first << ", result.second = " << result.second << std::endl;
    // del(key)接口，返回类型为bool，表示操作是否成功
    // 如果元素存在，那么返回true，如果元素不存在，那么返回false
    std::cout << "first delete: " << store.del("Yang") << std::endl;
    std::cout << "second delete: " << store.del("Yang") << std::endl;
}