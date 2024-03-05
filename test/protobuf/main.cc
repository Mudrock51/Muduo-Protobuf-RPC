#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

int test(){
    /*封装 LoginRequest 对象的数据*/
    LoginRequest req;
    req.set_name("Qin");
    req.set_pwd("123456");

    /*对象数据序列化->(char *)*/
    std::string send_str;
    if(req.SerializePartialToString(&send_str)){
        std::cout << send_str.c_str() << std::endl;
    }

    /*反序列化成一个 LoginRequest 对象*/
    LoginRequest reqBack;
    if(reqBack.ParseFromString(send_str)){
        std::cout << reqBack.name() << std::endl;
        std::cout << reqBack.pwd() << std::endl;
    }

    return 0;
}

int main(){
    LoginResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errorcode(1);
    rc->set_errmsg("登录失败");

    GetFriendListsResponse rspf;
    ResultCode *rt = rspf.mutable_result();
    rt->set_errorcode(0); 
    
    user *user1 = rspf.add_friendlist();
    user1->set_name("张三");
    user1->set_age(20);
    user1->set_sex(user::male);
    return 0;
}
