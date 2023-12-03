#pragma once

#include <cpprest/json.h>
#include <string>
#include <unordered_map>
#include "BinanceAccountItem.h"

using namespace std;


class BinanceAccountMgr {
public:
	static BinanceAccountMgr& GetInstance();
	~BinanceAccountMgr();
    BinanceAccountItem* GetAccountItem(int accountId);
    //void OnSubMessage(const web::json::value& content);
    void UpdateBySystem();
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
    string GetStrategyAccountStatus();
    string GetMarketRiskStatus();
    set<string> GetInstrumentList();
    vector<MsgCard> GetAlarmMsg();
    vector<MsgCard> GetFundingRateAlarmMsg();
    vector<MsgCard> GetPositionLiquidationPriceAlarmMsg();
    vector<igmonitor::RiskInfo> GetRiskInfo();
    vector<MsgCard> GetMarketStatusAlarmMsg();
    vector<MsgCard> GetOrderAlarmMsg();

private:
	BinanceAccountMgr();
    map<int, BinanceAccountItem*> mAccount;

    unordered_map<string, igmonitor::Asset> mTotalAsset;
    unordered_map<string, igmonitor::Position> mTotalPosition;
    unordered_map<string, igmonitor::MarketInfo> mMarketInfo;
    double openInterestAlarm;
};
