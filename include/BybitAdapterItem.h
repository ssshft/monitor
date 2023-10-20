#pragma once

#include "MonitorConfig.h"
#include "Bybit.h"
#include <map>
#include <set>

using namespace std;


class BybitAdapterItem {
public:
    BybitAdapterItem(AccountInfo info);
	~BybitAdapterItem();
    void UpdateAccountInfo();   
    set<string> GetInstrumentList();
    bybit::TotalAccountInfo& GetTotalAccountInfo();
    vector<bybit::Asset>& GetAsset();
    vector<bybit::Position>& GetPosition();
    vector<bybit::Order>& GetOpenOrder();
    double GetPositionValue(string asset);
    double GetFloatAmount(string asset);
    void GetLongShortFrozenPosition(string symbol, double longFrozenPos, double shortFrozenPos);
    int64_t GetUpdateTime();
    bool GetQueryStatus();
    vector<string>& GetQueryErrMsg();
    int isUnified();

private:
    AccountInfo accountInfo;
    Bybit* byb;
    bool enable;
    bybit::TotalAccountInfo totalAccountInfo;
    vector<bybit::Asset> vAsset;
    vector<bybit::Position> vPosition;
    vector<bybit::Order> vOpenOrder;
    string baseAsset;
    int64_t updateTime;

    bool query;
    vector<string> vQueryErrMsg;
};
