#include "mprpcchannel.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "mprpccontroller.h"
#include "socketmanager.h"
#include "zookeeperutil.h"


#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <google/protobuf/descriptor.h>
#include <iostream>
#include <memory>
#include <string>


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response, 
                          google::protobuf::Closure* done)
    {
        const google::protobuf::ServiceDescriptor* sd = method->service();
        std::string service_name = sd->name();      //service_name
        std::string method_name = method->name();   //method_name

        //set Serialize to args_str and get the length
        uint32_t args_size = 0;
        std::string args_str;       
        if (request->SerializeToString(&args_str))
        {
            args_size = args_str.size();
        }
        else
        {
            controller->SetFailed("serialize request error!");
            return;
        }

        //define rpcHeader
        mprpc::RpcHeader rpcHeader;
        rpcHeader.set_service_name(service_name);
        rpcHeader.set_method_name(method_name);
        rpcHeader.set_args_size(args_size);

        uint32_t header_size = 0;
        std::string rpc_header_str;
        if(rpcHeader.SerializeToString(&rpc_header_str))
        {
            header_size = rpc_header_str.size();
        }
        else 
        {
            controller->SetFailed("serialize rpc header error!");
            return;
        }

        //set the request:send_rpc_str
        std::string send_rpc_str;
        send_rpc_str.insert(0, std::string((char*)&header_size, 4)); //header_size
        send_rpc_str += rpc_header_str; //rpcheader
        send_rpc_str += args_str;       //args

        //Use tcp connection, to call the rpc Service
        //Use unique_ptr to apply RAII, manage the release of socket.
        int clientfd = socket(PF_INET, SOCK_STREAM, 0);
        std::unique_ptr<SocketManager> socketManager(new SocketManager(clientfd));
        if (clientfd == -1)
        {
            char errtxt[512] = {0};
            sprintf(errtxt, "create socket error! errno:%d", errno);
            controller->SetFailed(errtxt);
            exit(EXIT_FAILURE);
        }

        //find the ip:port from znode
        ZkClient zkCli;
        zkCli.start();
        std::string method_path = "/" + service_name + "/" + method_name;
        std::string host_data = zkCli.GetData(method_path.c_str());  //127.0.0.1:8000

        if (host_data == "")
        {
            controller->SetFailed(method_name + "is not exists");
            return;
        }
        int index = host_data.find(":");
        if (index == -1)
        {
            controller->SetFailed(method_path + "address is invalid!");
            return;
        }

        //delimit the host_data to ip:port
        std::string ip = host_data.substr(0, index);
        uint16_t port = atoi(host_data.substr(index + 1, host_data.size() - index).c_str());

        struct sockaddr_in server_addr;
        bzero(&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);
        server_addr.sin_port = htons(port);

        //Connect rpc Service
        if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
        {
            char errtxt[512] = {0};
            sprintf(errtxt, "connect error! errno:%d", errno);
            controller->SetFailed(errtxt);
            exit(EXIT_FAILURE);
        }

        //Send rpc Request
        if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
        {
            char errtxt[512] = {0};
            sprintf(errtxt, "send error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }

        //Recv rpc Response
        char recv_buf[1024] = {0};
        int recv_size = 0;
        if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
        {
            char errtxt[512] = {0};
            sprintf(errtxt, "recv error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }

        //Unpackage the rpc response
        //*Use ParseFromString() will appear the bug: if '\0',then Parse Failed.
        //Use ParseFromArray() to deal with the bug.
        if(!response->ParseFromArray(recv_buf, recv_size))
        {
            char errtxt[512] = {0};
            sprintf(errtxt, "parse error! response_str:%s", recv_buf);
            controller->SetFailed(errtxt);
            return;
        }
    }