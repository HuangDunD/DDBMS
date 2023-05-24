#pragma once

#include "transaction_manager.h"
#include <string>

struct PrepareResult
{
    txn_id_t txn_id;
    bool prepare_res;
};

class CoordinatorBackup
{
private:
    // file name for CoordinatorBackup file
    std::string coor_backup_name_;
    // file stream for CoordinatorBackup file
    std::fstream coor_backup_file_;

public:
    CoordinatorBackup(std::string server_name){
        auto n = server_name.rfind('.');
        coor_backup_name_ = FLAGS_log_path + server_name.substr(0, n) + "_coor_backups.log";
        // open coor_backup file stream
        coor_backup_file_.open(coor_backup_name_, std::ios::binary | std::ios::in | std::ios::app | std::ios::out);
        if (!coor_backup_file_.is_open()) {
            // coor_backup is not opended, which means the file doesn't exist
            // then we create it
            coor_backup_file_.clear();
            // std::ios::in will fail us when the file is not exist 
            coor_backup_file_.open(coor_backup_name_, std::ios::binary | std::ios::trunc | std::ios::app | std::ios::out);
            coor_backup_file_.close();
            // reopen it with original mode
            coor_backup_file_.open(coor_backup_name_, std::ios::binary | std::ios::in | std::ios::app | std::ios::out);
            if (!coor_backup_file_.is_open()) {
                std::cerr << "failed to open coor_backup file, filename: " << coor_backup_name_.c_str();
            }
        }
    };
    ~CoordinatorBackup(){
        if (coor_backup_file_.is_open()) {
            coor_backup_file_.close();
        }
    };

    void WriteCoordinatorBackup(PrepareResult &pr) {
        char* buf = reinterpret_cast<char*>(&pr);
        coor_backup_file_.write(buf, sizeof(PrepareResult));
    }

    // 给定事务id, 从文件中查找出来备份中事务的prepare状态
    bool ReadBackwards(txn_id_t txn_id) {
        if (!coor_backup_file_.is_open()) {
            std::cout << "File is not open" << std::endl;
            return;
        }
        
        // Move the file pointer to the end of the file
        coor_backup_file_.seekp(0, std::ios::end);
        int64_t end_pos = coor_backup_file_.tellp();

        // Read the file backwards, line by line
        PrepareResult pr;
        for (int64_t pos = end_pos - sizeof(PrepareResult); pos >= 0; pos--) {
            coor_backup_file_.seekp(pos);
            coor_backup_file_.read(reinterpret_cast<char*>(&pr), sizeof(PrepareResult));
            
            if(txn_id == pr.txn_id){
                return pr.prepare_res;
            }
        }
        return false;
    }
};