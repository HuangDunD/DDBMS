#include "timestamp.h"

//timestamp test
int main(){
    Oracle o;
    std::atomic<bool> oracle_background_running = true;
    std::thread update([&]{o.start(std::ref(oracle_background_running));});
    auto last = o.getTimeStamp();
    long long count = 0;
    while (1)
    {
        count++;
        // auto t1 = std::chrono::high_resolution_clock::now();
        auto now = o.getTimeStamp();
        // auto t2 = std::chrono::high_resolution_clock::now();
        // std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << std::endl;
        if(now <= last) {
            std::cout << "now: " << now << "last:" << last << std::endl;
            std::cout << "error" << std::endl;
        }
        std::cout << "now: " << now << "last:" << last << std::endl;
        last = now;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        // if(count % 10000 == 0){
        //     std::cout << count << std::endl;
        // }
    }
    update.join();
    return 0;
}