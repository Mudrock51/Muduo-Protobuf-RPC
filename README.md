

# RPC远程过程调用框架

本项目是基于Muduo库，Protobuf库和Zookeeper库实现的Rpc框架(简称为Mprpc):

1. 基于Muduo库实现TCP连接，保证客户端和服务端之间的网络通信。
2. 基于Protobuf库实现传输数据的序列化和反序列化，通过NotifyService方法实现Rpc服务的发布以及调用CallMethod来实现数据的传输。（主要内容）
3. 基于Zookeeper库实现分布式协调服务，运用znode节点和watcher机制实现简单的分布式。

### 项目目录:

- bin目录存放编译后的客户端以及服务端，存放日志和IP地址端口号配置文件。
- build存放CMake编译文件
- example存放项目的使用案例，实现friendservice和userservice(user服务需要修改/example目录下的CMakeLists.txt文件)
- lib存放静态库和头文件
- src存放源码头文件和实现
- test存放对protobuf的简单使用
- build.sh一键编译脚本，实现CMake编译项目。
- README.md是对项目的简单介绍。

