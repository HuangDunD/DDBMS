#include <string>

class KVStoreAPI {
public:
    // dir表示数据保存的文件夹位置，第一次使用KVStore一定要保证该文件夹不存在或者为空
    explicit KVStoreAPI(const std::string &dir) {}

    // 禁用复制构造函数和复制赋值函数
    KVStoreAPI(const KVStoreAPI &) = delete;
    KVStoreAPI &operator=(const KVStoreAPI &) = delete;

    // 基本API

    // put(key, value)：接受两个string参数，其中key和value不能为空字符串
    // 要求：value不能为空""，因为value为空表示该键值对不存在
    virtual void put(const std::string &key, const std::string &value) = 0;

    // get(key):获得key，返回一个二元组pair<bool, std::string>
    // 前者为true表示get操作成功，否则get操作失败
    virtual std::pair<bool, std::string> get(const std::string &key) = 0;

    // del(key)：删除key，如果在memtable中则直接删除，如果不再memtable中，则会
    // 插入一个新的键值对，其中value为空字符串""
    virtual bool del(const std::string &key) = 0;

    // flush(): 强制将memtable刷入磁盘中
    virtual void flush() = 0;

    // reset():删除所有数据
    // 危险：删除所有数据
    virtual void reset() = 0;
};