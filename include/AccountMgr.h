#pragma once

#include <string>
#include <unordered_map>
#include "AccountItem.h"


class AccountMgr {
public:
	static AccountMgr& GetInstance();
	~AccountMgr();
    void Init(sm::SecurityManager* s);
    AccountItem* GetAccountItem(int accountId);
    void UpdateByAdapter();
    void ClearZero();
    void Clear();
    void CalculateTotalAsset();
    void CalculateExposure();
    void CalculateRiskInfo();
    void CalculateAccount();
    void StatisticAccount();
    string GetPhysicalOverView();
    string GetPhysicalAccountStatus();
    set<string> GetInstrumentList();
    vector<MsgCard> GetAlarmMsg();
    vector<MsgCard> GetFundingRateAlarmMsg();
    vector<MsgCard> GetPositionLiquidationPriceAlarmMsg();
    vector<igmonitor::RiskInfo> GetRiskInfo();

private:
	AccountMgr();
    map<int, AccountItem*> mAccount;

    unordered_map<string, igmonitor::Asset> mTotalAsset;
    unordered_map<string, igmonitor::Position> mTotalPosition;
    double openInterestAlarm;
};
