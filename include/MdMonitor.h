#pragma once

#include <cpp_redis/cpp_redis>
#include <string>
#include <functional>
#include <cpprest/json.h>
#include <algorithm>
#include "Utility.h"

using namespace std;

class MdMonitor {
public:
    ~MdMonitor();
    static MdMonitor& GetInstance();
    void ConnectRedis();
    void Subscribe();
    void MaintainRedisConnected();
    static void OnSubMessage(const string& channel, const string& msg);
    void DealSubData();
    unordered_map<string, string>& GetCurrentStatus();
    vector<MsgCard> GetAlarmMsg();
    vector<string>& GetCurrentChannel();

private:
    MdMonitor();
    cpp_redis::subscriber redisSub;
    thread* maintainRedisConnected;
    thread* dealSubData;
    std::function<void(const string& channel, const string& msg)> f;
    bool isConnected;
    bool maintainFlag;
    bool dealSubFlag;
    vector<string> vCurrentChannel;
    unordered_map<string, string> mMdStatus;
};