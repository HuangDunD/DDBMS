#include "meta_server.h"

void MetaServer::open_meta_server(const std::string &meta_name){
    // struct stat st; 
    if( !is_dir(meta_name) ){
        throw MetaServerErrorException(MetaServerError::NO_META_DIR);
    }
    if (chdir(meta_name.c_str()) < 0) {
        throw MetaServerErrorException(MetaServerError::UnixError);
    }
    // Load meta
    // 打开一个名为DB_META_NAME的文件
    std::ifstream ifs (META_SERVER_FILE_NAME);
    // 将ofs打开的DB_META_NAME文件中的信息，按照定义好的operator>>操作符，读出到db_中
    ifs >> *this;  // 注意：此处重载了操作符>>
}

void MetaServer::close_meta_server(const std::string &meta_name){
    // struct stat st; 
    if( !is_dir(meta_name) ){
        throw MetaServerErrorException(MetaServerError::NO_META_DIR);
    }
    if (chdir(meta_name.c_str()) < 0) {
        throw MetaServerErrorException(MetaServerError::UnixError);
    }
    //Save Meta
    std::ofstream ofs (META_SERVER_FILE_NAME);
    ofs << *this; 
}

std::string MetaServer::getPartitionKey(std::string db_name, std::string table_name){
    std::shared_lock<std::shared_mutex> ms_latch(mutex_); 
    DbMetaServer *dms = db_map_[db_name];
    if(dms == nullptr) 
        throw MetaServerErrorException(MetaServerError::NO_DATABASE);
    std::shared_lock<std::shared_mutex> dms_latch(dms->get_mutex());
    ms_latch.unlock();
    
    TabMetaServer *tms = dms->gettablemap()[table_name];
    if(tms == nullptr)
        throw MetaServerErrorException(MetaServerError::NO_TABLE);
    std::shared_lock<std::shared_mutex> tms_latch(tms->mutex_);
    dms_latch.unlock();
    
    std::string res = tms->partition_key_name;
    return res;
}

Column_info MetaServer::GetColInfor(std::string db_name, std::string table_name){
    std::shared_lock<std::shared_mutex> ms_latch(mutex_); 
    DbMetaServer *dms = db_map_[db_name];
    if(dms == nullptr) 
        throw MetaServerErrorException(MetaServerError::NO_DATABASE);
    std::shared_lock<std::shared_mutex> dms_latch(dms->get_mutex());
    ms_latch.unlock();
    
    TabMetaServer *tms = dms->gettablemap()[table_name];
    if(tms == nullptr)
        throw MetaServerErrorException(MetaServerError::NO_TABLE);
    std::shared_lock<std::shared_mutex> tms_latch(tms->mutex_);
    dms_latch.unlock();
    
    auto res = tms->col_info;
    return res;
}

std::unordered_map<partition_id_t,ReplicaLocation> MetaServer::getReplicaLocationList
            (std::string db_name, std::string table_name, std::string partitionKeyName, std::string min_range, std::string max_range){
    
    std::shared_lock<std::shared_mutex> ms_latch(mutex_); 
    DbMetaServer *dms = db_map_[db_name];
    if(dms == nullptr) 
        throw MetaServerErrorException(MetaServerError::NO_DATABASE);
    std::shared_lock<std::shared_mutex> dms_latch(dms->get_mutex());
    ms_latch.unlock();
    
    TabMetaServer *tms = dms->gettablemap()[table_name];
    if(tms == nullptr)
        throw MetaServerErrorException(MetaServerError::NO_TABLE);
    std::shared_lock<std::shared_mutex> tms_latch(tms->mutex_);
    dms_latch.unlock();

    if(partitionKeyName != tms->partition_key_name)
        throw MetaServerErrorException(MetaServerError::PARTITION_KEY_NOT_TRUE);
    
    if(tms->partition_type == PartitionType::RANGE_PARTITION){
        return getReplicaLocationListByRange(tms, min_range, max_range);
    }
    else if(tms->partition_type == PartitionType::HASH_PARTITION){
        return getReplicaLocationListByHash(tms, min_range, max_range);
    }
    else{
        std::unordered_map<partition_id_t,ReplicaLocation> res;
        for(auto par : tms->partitions){
            auto rep_ptr = tms->table_location_.getReplicaLocation(par.p_id);
            if(rep_ptr == nullptr)
                throw MetaServerErrorException(MetaServerError::NO_PARTITION_OR_REPLICA);
            res[par.p_id] = *rep_ptr;
        }
        return res;
    }
}

std::unordered_map<partition_id_t,ReplicaLocation> MetaServer::getReplicaLocationListByRange ( 
    TabMetaServer *tms, std::string min_range, std::string max_range ){
    
    std::unordered_map<partition_id_t,ReplicaLocation> res;
    if(tms->partition_key_type == ColType::TYPE_INT || tms->partition_key_type == ColType::TYPE_FLOAT){
        for(auto par : tms->partitions){
            if(stoi(par.string_range.min_range) <= stoi(max_range)
                && stoi(par.string_range.max_range) >= stoi(min_range)){

                auto rep_ptr = tms->table_location_.getReplicaLocation(par.p_id);
                if(rep_ptr == nullptr)
                    throw MetaServerErrorException(MetaServerError::NO_PARTITION_OR_REPLICA);
                res[par.p_id] = *rep_ptr;
            }
        }
    }
    else if(tms->partition_key_type == ColType::TYPE_STRING){
        for(auto par : tms->partitions){
            if(par.string_range.min_range <= max_range
                && par.string_range.max_range >= min_range){

                auto rep_ptr = tms->table_location_.getReplicaLocation(par.p_id);
                if(rep_ptr == nullptr)
                    throw MetaServerErrorException(MetaServerError::NO_PARTITION_OR_REPLICA);
                res[par.p_id] = *rep_ptr;
            }
        }
    }

    return res;
}

std::unordered_map<partition_id_t,ReplicaLocation>  MetaServer::getReplicaLocationListByHash 
            (TabMetaServer *tms, std::string min_range, std::string max_range){
    //这里暂时先简化 由于Hash分区均匀分区的特征 对于一般的范围的查询 
    //极大概率是对于所有的分区都有分区涉及 因此可以不进行Hash计算直接返回所有分区
    //而对于等值查询可以直接定位到分区

    std::unordered_map<partition_id_t,ReplicaLocation> res;
    if(tms->partition_key_type == ColType::TYPE_INT || tms->partition_key_type == ColType::TYPE_FLOAT){
        int int_min_range = stoi(min_range);
        int int_max_range = stoi(max_range);

        if(int_max_range == int_min_range){
            //等值查询
            partition_id_t p_id = tms->HashPartition(int_min_range);
            auto rep_ptr = tms->table_location_.getReplicaLocation(p_id);
            if(rep_ptr == nullptr)
                throw MetaServerErrorException(MetaServerError::NO_PARTITION_OR_REPLICA);
            res[p_id] = *rep_ptr;
        }else{
            //范围查询
            for(auto par : tms->partitions){
                auto rep_ptr = tms->table_location_.getReplicaLocation(par.p_id);
                if(rep_ptr == nullptr)
                    throw MetaServerErrorException(MetaServerError::NO_PARTITION_OR_REPLICA);
                res[par.p_id] = *rep_ptr;
            }
        }
        return res;
    }
    else if(tms->partition_key_type == ColType::TYPE_STRING){
        if(max_range == min_range){
            //等值查询
            partition_id_t p_id = tms->HashPartition(min_range);
            auto rep_ptr = tms->table_location_.getReplicaLocation(p_id);
            if(rep_ptr == nullptr)
                throw MetaServerErrorException(MetaServerError::NO_PARTITION_OR_REPLICA);
            res[p_id] = *rep_ptr;
        }else{
            //范围查询
            for(auto par : tms->partitions){
                auto rep_ptr = tms->table_location_.getReplicaLocation(par.p_id);
                if(rep_ptr == nullptr)
                    throw MetaServerErrorException(MetaServerError::NO_PARTITION_OR_REPLICA);
                res[par.p_id] = *rep_ptr;
            }
        }

        return res;
    }

    return res;
}

bool MetaServer::UpdatePartitionLeader(std::string db_name, std::string tab_name, partition_id_t p_id, std::string ip_addr){

    std::shared_lock<std::shared_mutex> ms_latch(mutex_); 
    DbMetaServer *dms = db_map_[db_name];
    if(dms == nullptr) 
        throw MetaServerErrorException(MetaServerError::NO_DATABASE);
    std::shared_lock<std::shared_mutex> dms_latch(dms->get_mutex());
    ms_latch.unlock();
    
    TabMetaServer *tms = dms->gettablemap()[tab_name];
    if(tms == nullptr)
        throw MetaServerErrorException(MetaServerError::NO_TABLE);
    //上写锁
    std::unique_lock<std::shared_mutex> tms_latch(tms->mutex_);
    dms_latch.unlock();

    auto tab_loc = (*tms).table_location_;
    if(tab_loc.get_duplicate_type() != DuplicateType::DUPLICATE){
        return false;
    }
    size_t i = 0;
    for(; i < tab_loc.get_partition_list().size(); i++){
        if(tab_loc.get_partition_list()[i].get_partition_id() == p_id)
            break;
    }
    if(i >= tab_loc.get_partition_list().size()){
        return false;
    }
    for(auto &repli_loc : tab_loc.mutable_partition_list()[i].get_replica_location_vec()){
        if(repli_loc.ip_addr_ == ip_addr){
            repli_loc.role_ = Replica_Role::Leader;
        }
        else{
            repli_loc.role_ = Replica_Role::Follower;
        }
    }
    return true;
}