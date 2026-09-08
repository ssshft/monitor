#pragma once

#include "MonitorConfig.h"
#include "okx/OkxClient.h"
#include "securitymanager.h"
#include <map>
#include <set>


class OkxAdapterItem {
public:
    OkxAdapterItem(AccountInfo info, sm::SecurityManager* s);
	~OkxAdapterItem();
    void UpdateAccountInfo();   
    std::unordered_map<std::string, md::InstrumentInfo> GetInstrumentList();
    std::vector<okx::OkxAsset>& GetAsset();
    std::vector<okx::OkxPosition>& GetPosition();
    std::vector<okx::OkxOrder>& GetOpenOrder();
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

    OkxClient* okxClient;
    std::vector<okx::OkxAsset> vAsset;
    std::vector<okx::OkxPosition> vPosition;
    std::vector<okx::OkxOrder> vOpenOrder;
    std::vector<okx::OkxOrder> vOrder;
    std::string baseAsset;
    int64_t updateTime;
    bool query;
    std::vector<std::string> vQueryErrMsg;

    sm::SecurityManager* smc;
};
