#pragma once

#include "MonitorConfig.h"
#include "bybit/Bybit.h"
#include "securitymanager.h"
#include <map>
#include <set>


class BybitAdapterItem {
public:
    BybitAdapterItem(AccountInfo info, sm::SecurityManager* s);
	~BybitAdapterItem();
    void UpdateAccountInfo();   
    std::unordered_map<std::stirng, md::InstrumentInfo> GetInstrumentList();
    bybit::TotalAccountInfo& GetTotalAccountInfo();
    std::vector<bybit::Asset>& GetAsset();
    std::vector<bybit::Position>& GetPosition();
    std::vector<bybit::Order>& GetOpenOrder();
    double GetPositionValue(std::string asset);
    double GetFloatAmount(std::string asset);
    void GetLongShortFrozenPosition(std::string symbol, double longFrozenPos, double shortFrozenPos);
    int64_t GetUpdateTime();
    bool GetQueryStatus();
    std::vector<std::string>& GetQueryErrMsg();
    int isUnified();

private:
    AccountInfo accountInfo;
    std::unordered_map<std::stirng, md::InstrumentInfo> mInst;

    Bybit* byb;
    bool enable;
    bybit::TotalAccountInfo totalAccountInfo;
    std::vector<bybit::Asset> vAsset;
    std::vector<bybit::Position> vPosition;
    std::vector<bybit::Order> vOpenOrder;
    std::string baseAsset;
    int64_t updateTime;

    bool query;
    std::vector<std::string> vQueryErrMsg;

    sm::SecurityManager* smc;
};
