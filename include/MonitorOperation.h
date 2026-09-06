#pragma once
#include "securitymanager.h"



class MonitorOperation {

public:
    MonitorOperation();
    ~MonitorOperation();
    bool preStart();
    void run();

    void GetExchangeMd();
    void UpdateAccountInfo();
    void RunAccountMonitor();

private:

    sm::SecurityManager* smc;

    bool sendAccountAlarm;
    bool storeRiskInfo;
    std::unordered_map<std::string, int64_t> mAccountAlarmSendTime;

    int calcAccountStatsInterval;
    int accountAlarmInterval;
    int storeRiskInfoInterval;
    int systemMonitorInterval;

    int64_t startTime;
    int64_t lastCalcAccountStatsTime;
    int64_t lastAccountAlarmTime;
    int64_t lastStoreRiskInfoTime;
    int64_t lastSystemMonitorTime;

};
