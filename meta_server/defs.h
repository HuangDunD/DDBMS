#pragma once
#include <exception>
#include <string>
#include "dbconfig.h"

enum class MetaServerError {
    NO_DATABASE,
    NO_TABLE,
    PARTITION_KEY_NOT_TRUE,
    PARTITION_TYPE_NOT_TRUE,
    NO_PARTITION_OR_REPLICA,
    NO_META_DIR,
    UnixError,
};

class MetaServerErrorException : public std::exception
{
private:
    MetaServerError err_;
public:
    explicit MetaServerErrorException(MetaServerError err):err_(err){}
    MetaServerError getMetaServerError(){return err_;}
    std::string GetInfo() {
    switch (err_) {
        case MetaServerError::NO_DATABASE:
            return "there isn't this database in MetaServer";
        case MetaServerError::NO_TABLE:
            return "there isn't this table in MetaServer";
        case MetaServerError::PARTITION_KEY_NOT_TRUE:
            return "request's partitition key is different from Metaserver's";
        case MetaServerError::PARTITION_TYPE_NOT_TRUE:
            return "request's partitition type is different from Metaserver's";
        case MetaServerError::NO_PARTITION_OR_REPLICA:
            return "there isn't partition or replica required in MetaServer";
        case MetaServerError::NO_META_DIR:
            return "no meta server dir found";
        case MetaServerError::UnixError: 
            return "cd meta server dir error";
    }
    return "";
  }
    ~MetaServerErrorException(){};
};