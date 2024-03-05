#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <iostream>
#include <zookeeper/zookeeper.h>
#include "logger.h"
#include "zookeeperutil.h"


/*框架提供给外部使用的, 可以发布rpc方法的函数接口*/
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    /*获取了服务对象的描述信息*/
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();

    /*获取服务的名字*/
    std::string service_name = pserviceDesc->name();
    /*获取服务对象方法的数量*/
    int methodCnt = pserviceDesc->method_count();
    LOG_INFO("service_name:%s", service_name.c_str());

    for(int i = 0; i < methodCnt; ++i){
        //* 获取了服务对象指定下标的服务方法的描述(抽象描述)
        //const MethodDescriptor* method(int index) const;
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        LOG_INFO("method_name:%s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

/*启动Rpc服务节点, 提供远程网络调用服务*/
void RpcProvider::Run()
{
    // Get ip/port according to Config.Load
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    /*创建TcpServer对象*/
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    /*绑定连接回调和消息读写回调方法*/
    //分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1,
            std::placeholders::_2, std::placeholders::_3));

    /*设置muduo库的线程数量*/
    server.setThreadNum(4); //1个IO线程, 多个work线程处理: Reactor
    
    //把当前rpc节点发布到服务注册到zk上, 让rpc Client可以从zk发现服务
    //session timeout 30s
    //zkClient 1/3 * timeout 发送ping消息
    ZkClient zkCli;
    zkCli.start();
    //service_name永久性节点, method_name临时性节点
    for(auto &sp : m_serviceMap)
    {   
        // /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);

        for(auto &mp : sp.second.m_methodMap)
        {   
            // /UserServiceRpc/method 存储当前rpc服务节点的ip:port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            zkCli.Create(method_path.c_str(), method_path_data, 
                        strlen(method_path_data), ZOO_EPHEMERAL); //临时性节点
        }
    }

    std::cout << "RpcProvider start service at ip:"
        << ip << " port:" << port << std::endl;
    

    server.start(); // 启动网络服务

    /*启动Epoll_wait()*/
    m_eventLoop.loop();
}

// a new socket connectionCall
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn) 
{
    if (!conn->connected())
    {
        //close the connection
        conn->shutdown();
    }
}

/**
 * @brief 
 * 在框架内部, RpcProvider和RpcConsumer协商好通信的protobuf数据类型
 * service_name、method_name、args
 * 定义 message 类型, 进行数据头的序列化和反序列化
 * service_name method_name args_size // 记录数据和参数, 同时考虑粘包问题
 * 16UserServiceLoginzhang san123456

 * header_size(4bytes) + header_str + args_str;
 * std::string insert/copy;
 */

/*已建立连接用户的读写事件回调*/
// OnMessage 方法相应远程rpc服务的调用请求
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, 
                            muduo::net::Buffer* buffer, 
                            muduo::Timestamp)
{
    //网络上接收的远程rpc调用请求的字符串
    std::string recv_buf = buffer->retrieveAllAsString();

    //如何读取字符串的前4个字节(一个整数)?
    //直接在内存地址中处理
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0); //从0开始,指定4个字节放入header_size里

    //根据header_size读取数据头的原始字符流, 反序列化数据得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size); //从第4个字节开始, 读取header_size长度的子串
    mprpc::RpcHeader rpcHeader;
    std::string service_name, method_name;
    uint32_t args_size;

    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        //数据头反序列化失败
        LOG_ERR("rpc_header_str:%s, PARSE ERROR!",rpc_header_str.c_str());
        return;
    }

    //获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(header_size + 4, args_size);

    //打印调试信息
    LOG_INFO("=============打印调试信息==============");
    LOG_INFO("header_size:%d", header_size);
    LOG_INFO("rpc_header_str:%s", rpc_header_str.c_str());
    LOG_INFO("service_name:%s", service_name.c_str());
    LOG_INFO("method_name:%s", method_name.c_str());
    LOG_INFO("args_str:%s", args_str.c_str());
    LOG_INFO("=====================================");

    //获取Service对象和Method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        LOG_ERR("%s, IS NOT EXIST!",service_name.c_str());
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        LOG_ERR("%s: %s IS NOT EXIST!",service_name.c_str(), method_name.c_str());
        return;
    }

    google::protobuf::Service *service = it->second.m_service;        //UserService
    const google::protobuf::MethodDescriptor *method = mit->second;   //Method

    /*生成rpc方法调用的请求request和响应response*/
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        LOG_ERR("REQUEST PARSE ERROR! CONTENT:%s",args_str.c_str());
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();
    /*绑定一个 Closure的回调函数*/
    google::protobuf::Closure *done = google::protobuf::NewCallback //通过NewCallback返回一个对象
        <RpcProvider, const muduo::net::TcpConnectionPtr&, google::protobuf::Message*>
            (this, &RpcProvider::SendRpcResponse, conn, response);

    // 在框架上根据远端rpc请求调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}


// Closure的回调操作, 用于序列化rpc的响应和发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    //Serialize the response
    std::string response_str;
    if (response->SerializeToString(&response_str)) 
    {
        //Serialize success, send the execuate results to caller
        conn->send(response_str);
        LOG_INFO("SERIALIZETOSTRING success");
    }
    else
    {
        LOG_ERR("SERIALIZE RESPONSE_STR ERROR!");
    }
    conn->shutdown(); //模拟 http 的短链接服务, 由rpcprovider主动断开连接
}