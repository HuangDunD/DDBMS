#include <vector>
#include <stdint.h>
#include "partition_location.h"

enum class DuplicateType : int64_t
{
  NOT_DUPLICATE = 0, //非复制表
  DUPLICATE         //复制表
};

class PhyTableLocation {
public:
    PhyTableLocation(){};
    ~PhyTableLocation(){};
    PhyTableLocation(table_oid_t table_oid, DuplicateType duplicate_type, 
                        std::vector<PhyPartitionLocation> partiton_list):
              table_oid_(table_oid), duplicate_type_(duplicate_type), partiton_list_(std::move(partiton_list)){};

    ReplicaLocation* getReplicaLocation(partition_id_t p_id){
      std::vector<PhyPartitionLocation>::iterator iter = partiton_list_.begin();
      for(;iter != partiton_list_.end(); iter++){
        if(iter->get_partition_id() == p_id)
          break;
      }
      if(iter == partiton_list_.end())
        return nullptr;
      return iter->get_replica_location(Replica_Role::Leader);
    }

    inline table_oid_t get_table_id() const {return table_oid_;}
    inline void set_table_id(table_oid_t tab_id) {table_oid_ = tab_id;}

    inline DuplicateType get_duplicate_type() const { return duplicate_type_;}
    inline void set_duplicate_type(DuplicateType type) {duplicate_type_ = type;}

    inline const std::vector<PhyPartitionLocation>& get_partition_list() {return partiton_list_;}
    inline std::vector<PhyPartitionLocation>& mutable_partition_list() {return partiton_list_;}

    friend std::ostream &operator<<(std::ostream &os, const PhyTableLocation &phytabloc) {
        os << phytabloc.table_oid_ << ' ' << phytabloc.duplicate_type_ << ' ' << phytabloc.partiton_list_.size() << '\n';
        for (auto &entry : phytabloc.partiton_list_){
            os << entry;
        }
        return os;
    }

    friend std::istream &operator>>(std::istream &is, PhyTableLocation &phytabloc) {
        size_t n;
        is >> phytabloc.table_oid_ >> phytabloc.duplicate_type_ >> n;
        for(size_t i=0; i<n; i++){
            PhyPartitionLocation phyparloc;
            is >> phyparloc;
            phytabloc.partiton_list_.push_back(phyparloc);
        }
        return is;
    }

private:

    table_oid_t table_oid_; //表id

    DuplicateType duplicate_type_; //是否为复制表

    std::vector<PhyPartitionLocation> partiton_list_; //表中各个分区所在的位置

};