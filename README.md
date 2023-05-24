# RucDDBMS

RucDDBMS是一个精简的分布式数据库原型系统，存储层采用了KV键值存储，同时使用了Raft协议进行多副本间日志的复制，事务层采用了2PL+2PC算法。

## 实验环境：
- 操作系统：Ubuntu 18.04 及以上(64位)
- 编译器：GCC
- 编程语言：C++17
- 管理工具：cmake
- 推荐编辑器：VScode

### 依赖环境库配置：
- gcc 7.1及以上版本（要求完全支持C++17）
- cmake 3.16及以上版本
- readline

欲查看有关依赖运行库和编译工具的更多信息，以及如何运行的说明，请查阅[RucDDBMS使用文档](docs/使用文档.md)

### 开发规范文档

- [开发文档](docs/开发文档.md)

### 项目说明文档

- [RucDDBMS环境配置文档](docs/环境配置.md)
- [RucDDBMS使用文档](docs/使用文档.md)

### 学生实验文档(2023-05-24日更新)

> 请使用命令git pull来拉取最新的实验文档

- [RucDDBMS大作业文档](docs/大作业文档.md)
