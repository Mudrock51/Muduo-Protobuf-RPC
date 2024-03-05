#include <iostream>
#include <string>
#include "../user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"


/**
 * 原来是一个本地服务, 提供两个进程内的本地方法 Login/GetFriendLists
 */

class UserService : public fixbug::userServiceRpc{ // 使用在rpc服务发布端
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "Logining..." << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "Doing local service: Register" << std::endl;
        std::cout << "id: " << id << " name: " << name
                << " pwd: "<< pwd << std::endl;
        return true;
    }

    /*重写基类Rpc虚函数, 框架直接调用*/
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        //框架给业务上报请求参数 LoginRequest, 应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool login_result = Login(name, pwd); // 本地业务

        //把响应写入框架, 包括错误码, 错误消息和返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errorcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        //执行回调
        /*执行相应对象数据的序列化和网络发送*/
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id, name, pwd);
        
        response->mutable_result()->set_errorcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        done->Run();
    }
};


int main(int argc, char* argv[]){
    /*调用框架的初始化操作*/
    MprpcApplication::Init(argc, argv);

    /* provider是一个rpc网络服务对象,把UserService对象发布到rpc节点*/
    RpcProvider provider;
    provider.NotifyService(new UserService());
    // provider.NotifyService(new ProductService());

    /*启动一个rpc服务发布节点, Run以后, 进程计入阻塞状态,等待远程rpc调用请求*/
    provider.Run(); // 进入阻塞

    return 0; 
}