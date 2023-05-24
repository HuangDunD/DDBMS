#include <iostream>
#include <string>

#include "KVStore.h"

int main()
{
    KVStore store = KVStore("./data");
    std::cout << "Start Putting 65536 kv pairs" << std::endl;
    for(uint64_t i = 0; i <= 65536; i++){
        store.put(i, "Yang Harvey");
    }
    std::cout << "End Putting 65536 kv pairs" << std::endl;
    std::string result = store.get(1);
    std::cout << result << std::endl;
    return 0;
}