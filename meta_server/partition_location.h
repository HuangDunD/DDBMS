#include <string>
#include <iostream>

using table_oid_t = int32_t;
using partition_id_t = int32_t;

template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
std::ostream &operator<<(std::ostream &os, const T &enum_val) {
    os << static_cast<int>(enum_val);
    return os;
}

template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
std::istream &operator>>(std::istream &is, T &enum_val) {
    int int_val;
    is >> int_val;
    enum_val = static_cast<T>(int_val);
    return is;
}

enum Replica_Role{
    INVALID_ROLE = 0,
    Leader,
    Follower
};

struct ReplicaLocation
{
    std::string ip_addr_;
    int32_t port_;
    Replica_Role role_;

    friend std::ostream &operator<<(std::ostream &os, const ReplicaLocation &repli_loc) {
        os << repli_loc.ip_addr_ << ' ' << repli_loc.port_ << ' ' << repli_loc.role_ << '\n'; 
        return os;
    }

    friend std::istream &operator>>(std::istream &is, ReplicaLocation &repli_loc) {
        is >> repli_loc.ip_addr_ >> repli_loc.port_  >>repli_loc.role_ ; 
        return is;
    }
};

class PhyPartitionLocation
{
public:
    inline table_oid_t get_table_id() const {return table_oid_;}

    inline partition_id_t get_partition_id() const {return p_id_;}

    inline int32_t get_replica_cnt() const {return replica_cnt_;}

    inline std::vector<ReplicaLocation>& get_replica_location_vec() {return repliaca_location_;}
    
    inline ReplicaLocation* get_replica_location(Replica_Role role){
        for(auto &replica_loc: repliaca_location_){
            if(replica_loc.role_ == role)
                return &replica_loc;
        }
        return nullptr;
    }

    friend std::ostream &operator<<(std::ostream &os, const PhyPartitionLocation &phyparloc) {
        os << phyparloc.table_oid_ << ' ' << phyparloc.p_id_ << ' ' << phyparloc.replica_cnt_ 
                << ' ' << phyparloc.repliaca_location_.size() << '\n';
        for (auto &entry : phyparloc.repliaca_location_){
            os << entry;
        }
        return os;
    }

    friend std::istream &operator>>(std::istream &is, PhyPartitionLocation &phyparloc) {
        size_t n;
        is >> phyparloc.table_oid_ >> phyparloc.p_id_ >> phyparloc.replica_cnt_ >> n;
        for(size_t i=0; i<n; i++){
            ReplicaLocation repli_loc;
            is >> repli_loc;
            phyparloc.repliaca_location_.push_back(repli_loc);
        }
        return is;
    }

    PhyPartitionLocation(){};

    PhyPartitionLocation(table_oid_t table_oid, partition_id_t p_id, int32_t replica_cnt, 
                                        std::vector<ReplicaLocation> repliaca_location)
                :table_oid_(table_oid), p_id_(p_id), replica_cnt_(replica_cnt),repliaca_location_(std::move(repliaca_location)){};  

    ~PhyPartitionLocation(){};

private:
    table_oid_t table_oid_; //表id
    partition_id_t p_id_; //分区id
    int32_t replica_cnt_; //副本数
    std::vector<ReplicaLocation> repliaca_location_; //副本所在位置

};
