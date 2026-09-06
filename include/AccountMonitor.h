#pragma once

#include "Utility.h"

class AccountMonitor {
public:
    ~AccountMonitor();
    static AccountMonitor& GetInstance();
    void DealSubData();
    unordered_map<string, string>& GetCurrentStatus();
    vector<MsgCard> GetAlarmMsg();
    vector<MsgCard> GetFundingRateAlarmMsg();
    
private:
    AccountMonitor();
    vector<AccountMonitorItem*> vAccountMonitorItem;
    thread* dealSubData;
    bool dealSubFlag;
    unordered_map<string, string> mAccountStatus;
};