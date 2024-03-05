#pragma once
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

/*单例模式*/
/* mprpc框架的初始化类 */
class MprpcApplication{
public:
    static void Init(int argc, char* argv[]);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
    
private:
    static MprpcConfig m_config;
    static bool m_init;

    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};  