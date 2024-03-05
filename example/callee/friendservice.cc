#include <cstdint>
#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <sys/types.h>
#include <vector>
#include "logger.h"

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid){
        std::cout << "do GetFriendsList service! userid: "<< userid << std::endl;
        std::vector<std::string> v;
        v.push_back("Yang xue feng");
        v.push_back("Tang di bing");
        v.push_back("Qin ji shen");
        return v;
        
    }

    void GetFriendsList(google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRequest* request,
                       ::fixbug::GetFriendsListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->id();

        std::vector<std::string> friendlist = GetFriendsList(userid);
        response->mutable_result()->set_errorcode(0);
        response->mutable_result()->set_errmsg("");
        for(std::string& name: friendlist){
            std::string *p = response->add_friends();
            *p = name;
        }
        
        done->Run();
    }
};


int main(int argc, char* argv[]){

    /*调用框架的初始化操作*/
    MprpcApplication::Init(argc, argv);

    /* provider是一个rpc网络服务对象,把FriendService对象发布到rpc节点*/
    RpcProvider provider;
    provider.NotifyService(new FriendService());
    // provider.NotifyService(new ProductService());

    /*启动一个rpc服务发布节点, Run以后, 进程计入阻塞状态,等待远程rpc调用请求*/
    provider.Run(); // 进入阻塞

    return 0; 
}