#pragma once

#include <cpp_redis/cpp_redis>
#include <string>


class MsgPub {
public:
    static MsgPub& GetInstance();
    ~MsgPub();
    void ConnectRedis();
    void MaintainRedisConnected();
    void Publish(const std::string& channel, const std::string& msg);

private:
    MsgPub();
    cpp_redis::client client;
    std::thread* maintainRedisConnected;
    bool isConnected;
    bool maintainFlag;
};