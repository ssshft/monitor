#pragma once

#include "MonitorConfig.h"
#include "Utility.h"
#include <cpp_redis/cpp_redis>
#include <cpprest/json.h>


using namespace std;


class BasicInfoMgr {
public:
    static BasicInfoMgr& GetInstance();
	~BasicInfoMgr();
    void ConnectRedis();
    void MaintainRedisConnected();
    void Init();
    InstrumentInfo& GetBasicInfo(string key);
    string& GetSysIdByOriginId(string instrumentKey);
    bool GetInstrumentInfo(string exchId, string instType, string instId, string& value);
    bool GetAllInstrumentInfo(string& value);
    void UpdateInstrumentInfo();

private:
	BasicInfoMgr();
    bool Get(const char* key, string& value);
    unordered_map<string, InstrumentInfo> mInstrumentInfo;
    unordered_map<string, string> mOriginIdSysId;
    cpp_redis::client client;
    bool isConnected;
    bool maintainFlag;
    thread* maintainRedisConnected;
};