#pragma once
#include <string>
#include <unordered_map>
#include <queue>
#include "Utility.h"

using namespace std;

class ProductItem {
public:
    ProductItem(string na, vector<int> v);
    ~ProductItem();
    void UpdateAsset(unordered_map<string, igmonitor::Asset>& source, unordered_map<string, igmonitor::Asset>& target);
    void CalculateRiskInfo();
    vector<MsgCard> GetAlarmMsg();
    vector<MsgCard> GetFundingRateAlarmMsg();
    void UpdateAccountInfo();
    bool GetAdapterQueryStatus();
    igmonitor::RiskInfo& GetRiskInfo();
private:
    string name;
    vector<int> vAccountId;
    unordered_map<string, igmonitor::Asset> mSpotAsset;
    unordered_map<string, igmonitor::Asset> mUFutureAsset;
    unordered_map<string, igmonitor::Asset> mCFutureAsset;
    unordered_map<string, igmonitor::Exposure> mExposure;
    unordered_map<string, igmonitor::PositionFundingRate> mPositionFundingRate;
    igmonitor::RiskInfo riskInfo;
    string baseAsset;
    AlarmInfo alarmInfo;
    bool adapterQuery;
    string exchangeStr;
    set<string> sSymbolPriceZero;

    unordered_map<string, int64_t> mFundingRateTime;
};
