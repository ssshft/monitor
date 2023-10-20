#pragma once
#include "Utility.h"
#include <cpp_redis/cpp_redis>
#include <string>
#include <functional>
#include <cpprest/json.h>


class AccountMonitorItem {
public:
    AccountMonitorItem(AccountMonitorInfo info);
    ~AccountMonitorItem();
    void ConnectRedis();
    void SubScirbe();
    void MaintainRedisConnected();
    static void OnSubMessage(const string& channel, const string& msg);
    
private:
    AccountMonitorInfo accountMonitorInfo;
    cpp_redis::subscriber redisSub;
    thread* maintainRedisConnected;
    
    bool isConnected;
    bool maintainFlag;
    
};