#pragma once

#include "MonitorConfig.h"
#include "gateio/GateioSpot.h"
#include "gateio/GateioPerpetual.h"
#include "gateio/GateioUnified.h"
#include "securitymanager.h"
#include <map>
#include <set>



class GateioAdapterItem {
public:
    GateioAdapterItem(AccountInfo info, sm::SecurityManager* s);
	~GateioAdapterItem();
    void UpdateAccountInfo();   
    std::unordered_map<std::string, md::InstrumentInfo> GetInstrumentList();
    std::vector<gateio::SpotAsset>& GetSpotAsset();
    std::vector<gateio::FutureAsset>& GetPerpetualAsset();
    std::vector<gateio::FuturePosition>& GetPerpetualPosition();
    std::vector<gateio::FutureOrder>& GetPerpetualOpenOrder();
    std::vector<gateio::FutureOrder>& GetPerpetualOrder();
    double GetPerpetualPositionValue(std::string asset);
    double GetPerpetualFloatAmount(std::string asset);
    double GetPerpetualAssetTotal(std::string asset);
    void GetPerpetualLongShortFrozenPosition(std::string symbol, double longFrozenPos, double shortFrozenPos);
    int64_t GetUpdateTime();
    bool GetQueryStatus();
    std::vector<std::string>& GetQueryErrMsg();
    int isUnified();

private:
    AccountInfo accountInfo;
    std::unordered_map<std::string, md::InstrumentInfo> mInst;

    GateioSpot* gateioSpot;
    GateioPerpetual* gateioPerpetual;
    GateioUnified* gateioUnified;
    bool spotEnable;
    bool perpetualEnable;
    bool unifiedEnable;
    std::vector<gateio::SpotAsset> vSpotAsset;
    std::vector<gateio::FutureAsset> vPerpetualAsset;
    std::vector<gateio::FuturePosition> vPerpetualPosition;
    std::vector<gateio::FutureOrder> vPerpetualOpenOrder;
    std::vector<gateio::FutureOrder> vPerpetualOrder;
    gateio::UnifyTotalAccount unifyTotalAccount;
    string baseAsset;
    int64_t updateTime;

    bool query;
    std::vector<std::string> vQueryErrMsg;

    sm::SecurityManager* smc;
};
