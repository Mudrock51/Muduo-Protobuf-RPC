#include "mprpcconfig.h"
#include <iostream>
#include <string>

// 解析
void MprpcConfig::LoadConfigFile(const char* config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (pf == nullptr){
        std::cout << config_file << "is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    /*处理:1."#", 2.正确的配置项=, 3."去掉多余的空格"*/
    while(!feof(pf)){
        char buf[512] = {0};
        fgets(buf, 512, pf);

        // 去掉字符串前面多余的空格
        std::string read_buf(buf);
        Trim(read_buf);

        // 判断#
        if (read_buf[0] == '#' || read_buf.empty()){
            continue;
        }

        // 解析配置项
        int index = read_buf.find('=');
        if(index == -1){
            // 配置不合法
            continue;
        }

        std::string key;
        std::string value;
        key = read_buf.substr(0, index);
        Trim(key); //rpcserviceip=127.0.0.1\n
        int endIndex = read_buf.find('\n', index);
        value = read_buf.substr(index + 1, endIndex - index - 1);
        Trim(value);
        m_configMap.insert({key, value});
    }
}

// 查询
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if(it == m_configMap.end()) return "";
    return it->second;
}


void MprpcConfig::Trim(std::string &src_buf)
{
    int index = src_buf.find_first_not_of(' ');
    if(index != -1){ // 字符串前面有空格
        src_buf = src_buf.substr(index, src_buf.size() - index);
    }

    index = src_buf.find_last_not_of(' ');
    if(index != -1){ // 字符串后面有空格
        src_buf = src_buf.substr(0, index + 1);
    }

}

