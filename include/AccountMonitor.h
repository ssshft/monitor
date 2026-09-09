#pragma once

#include "Utility.h"

class AccountMonitor {
public:
    ~AccountMonitor();
    static AccountMonitor& GetInstance();
    std::unordered_map<std::string, std::string>& GetCurrentStatus();
    std::vector<MsgCard> GetAlarmMsg();
    std::vector<MsgCard> GetFundingRateAlarmMsg();
    
private:
    AccountMonitor();
    std::unordered_map<std::string, std::string> mAccountStatus;
};