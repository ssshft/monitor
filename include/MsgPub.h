#pragma once

#include <cpp_redis/cpp_redis>
#include <cpprest/json.h>
#include <string>
#include <functional>

using namespace std;


class MsgPub {
public:
    static MsgPub& GetInstance();
    ~MsgPub();
    void ConnectRedis();
    void SubConnectRedis();
    void SubScirbe();
    void MaintainRedisConnected();
    void Publish(string channel, const string& msg);
    static void OnSubMessage(const string& channel, const string& msg);

private:
    MsgPub();
    cpp_redis::client client;
    cpp_redis::subscriber redisSub;
    thread* maintainRedisConnected;
    
    bool isConnected;
    bool maintainFlag;
};