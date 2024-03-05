#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"
#include "mprpccontroller.h"

int main(int argc, char* argv[]){
    // 程序启动以后, 想使用mprpc框架来享受rpc服务调用, 需要先调用框架的初始化函数
    MprpcApplication::Init(argc, argv); //只初始化一次

    //演示调用远程发布的rpc方法Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    //rpc方法的请求参数
    fixbug::GetFriendsListRequest request;
    request.set_id(1000);
    //rpc方法的响应
    fixbug::GetFriendsListResponse response;
    //发起rpc方法的调用, 同步rpc的调用过程
    MprpcController controller;

    //MprpcChannel::callmethod
    stub.GetFriendsList(&controller, &request, &response, nullptr); // RpcChannel->RpcChannel::callMethod

    //一次rpc调用完成, 读取调用的结果
    if(controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else 
    {
        if(0 == response.result().errorcode())
        {
            std::cout << "rpc GetFriendsLists response success:" << std::endl;
            int size = response.friends_size();
            for(int i = 0; i < size; i++){
                std::cout << "index: " << i + 1 << " name: " << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendsLists response error:" << response.result().errmsg() << std::endl;
        }
    }
    

    return 0;
}