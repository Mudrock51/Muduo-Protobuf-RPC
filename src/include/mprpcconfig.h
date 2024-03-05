#pragma once

#include <string>
#include <unordered_map>

/**
 * @brief 
 * rpcserverip, rpcserverport
 * zookeeperip, zookeeperport
 */
/*读取配置文件类框架*/
class MprpcConfig{
public:
    void LoadConfigFile(const char* config_file);    //负责加载配置文件
    std::string Load(const std::string &key);        //查询配置项信息
private:
    std::unordered_map<std::string, std::string> m_configMap;
    void Trim(std::string &src_buf);                 //去掉空格
};

