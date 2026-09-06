#pragma once

#include <cpprest/json.h>
#include <string>
#include <unordered_map>
#include "AccountItem.h"

using namespace std;


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
    vector<MsgCard> GetMarketStatusAlarmMsg();
    vector<MsgCard> GetOrderAlarmMsg();

private:
	AccountMgr();
    map<int, AccountItem*> mAccount;

    unordered_map<string, igmonitor::Asset> mTotalAsset;
    unordered_map<string, igmonitor::Position> mTotalPosition;
    unordered_map<string, igmonitor::MarketInfo> mMarketInfo;
    double openInterestAlarm;
};
