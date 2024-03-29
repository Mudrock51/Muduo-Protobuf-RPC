#include "mprpcapplication.h"
#include "mprpcconfig.h"
#include <iostream>
#include <unistd.h>

MprpcConfig MprpcApplication::m_config; //TODO
bool MprpcApplication::m_init = false;

void ShowArgsHelp(){
    std::cout << "format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char* argv[]){
    if (m_init) exit(EXIT_FAILURE);
    if(argc < 2){
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1){
        switch (c) {
            case 'i':
                config_file = optarg;
                break;
            case '?':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
                break;
            case ':':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    m_init = true;
    /*加载配置文件*/
    m_config.LoadConfigFile(config_file.c_str());
    
};


MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}


MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}
