#pragma once
#include "defs.h"

#include <chrono>
#include <string>
#include <sys/stat.h>
#include <fstream>
#include <atomic>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <iostream>

const int physicalShiftBits = 18; 
const int maxRetryCount = 100;
const uint64_t updateTimestampStep = 30;
const uint64_t saveTimestampInterval = 1;
const uint64_t updateTimestampGuard = 1;
const int64_t maxLogical = int64_t(1 << physicalShiftBits);
const std::string TIMESTAMP_FILE_NAME = "last_timestamp";

class Oracle
{
private:
    uint64_t lastTS;
    struct timestamp
    {
        std::mutex mutex_;
        uint64_t physiacl;
        uint64_t logical;
    };
    timestamp current_;
    
public:
    Oracle(){ lastTS = 0; };
    ~Oracle(){};
    void start(std::atomic<bool> &oracle_background_running){
        syncTimestamp();
        while(oracle_background_running){
            updateTimestamp();
            std::this_thread::sleep_for(std::chrono::milliseconds(updateTimestampStep));
        }
    }

    void getTimestampFromPath(){
        struct stat st; 
        if( ! (stat(DATA_DIR.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) ){
            throw MetaServerErrorException(MetaServerError::NO_META_DIR);
        }
        if (chdir(DATA_DIR.c_str()) < 0) {
            throw MetaServerErrorException(MetaServerError::UnixError);
        }
        //get TimeStamp
        std::ifstream ifs (TIMESTAMP_FILE_NAME);
        ifs >> lastTS; 
    }

    void saveTimeStampToPath(uint64_t ts){
        struct stat st; 
        if( ! (stat(DATA_DIR.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) ){
            throw MetaServerErrorException(MetaServerError::NO_META_DIR);
        }
        if (chdir(DATA_DIR.c_str()) < 0) {
            throw MetaServerErrorException(MetaServerError::UnixError);
        }
        //save TimeStamp
        std::ofstream ofs (TIMESTAMP_FILE_NAME);
        ofs << ts;
    }

    void syncTimestamp(){
        getTimestampFromPath();
        auto next = GetPhysical();
        if( next-lastTS < updateTimestampGuard){
            std::cerr << "system time may be incorrect" << std::endl;
            next = lastTS + updateTimestampGuard;
        }
        uint64_t save = next + saveTimestampInterval;
        saveTimeStampToPath(save);
        std::cout << "sync and save timestamp" << std::endl;
        std::unique_lock<std::mutex> latch_(current_.mutex_);
        current_.physiacl = next;
        current_.logical = 0;
        return ;
    }

    void updateTimestamp(){
        std::unique_lock<std::mutex> latch_(current_.mutex_);
        uint64_t physical = current_.physiacl;
        uint64_t logic = current_.logical;
        
        auto now = GetPhysical();
        int64_t jetLag = now - physical;
        uint64_t next;
        if(jetLag > signed(updateTimestampGuard)){
            next = now;
        }else if(logic > maxLogical/2){
            next = physical+1;
        }else{return;}
        if(signed(lastTS-next)<= signed(updateTimestampGuard)){
            uint64_t save = next + saveTimestampInterval;
            saveTimeStampToPath(save);
        }
        current_.physiacl = next;
        current_.logical = 0;
        return;
    }

    uint64_t getTimeStamp(){
        for(int i=0; i < maxRetryCount; i++){
            std::unique_lock<std::mutex> latch_(current_.mutex_);
            uint64_t physical = current_.physiacl;
            uint64_t logic = ++current_.logical;
            latch_.unlock();
            if(current_.physiacl == 0){
                std::cout << "we haven't synced timestamp ok, wait and retry." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                continue;
            }
            if(logic > maxLogical){
                std::cout << "logical part outside of max logical interval" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(updateTimestampStep));
                continue;
            }
            return ComposeTS(physical, logic);
        }
        return 0;
    }

    uint64_t ComposeTS(uint64_t physical, uint64_t logical){
        return uint64_t((physical << physicalShiftBits)+ logical);
    }

    uint64_t ExtractPhysical(uint64_t ts) {
        return uint64_t(ts >> physicalShiftBits); 
    }

    uint64_t GetPhysical() { 
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch());
        return ms.count();
    }

    
};
