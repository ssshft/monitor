#pragma once

#include "MonitorConfig.h"
#include "GateioSpot.h"
#include "GateioDelivery.h"
#include "GateioPerpetual.h"
#include "GateioCrossMargin.h"
#include <map>
#include <set>

using namespace std;


class GateioAdapterItem {
public:
    GateioAdapterItem(AccountInfo info);
	~GateioAdapterItem();
    void UpdateAccountInfo();   
    set<string> GetInstrumentList();
    vector<gateio::SpotAsset>& GetSpotAsset();
    vector<gateio::FutureAsset>& GetDeliveryAsset();
    vector<gateio::FuturePosition>& GetDeliveryPosition();
    vector<gateio::FutureOrder>& GetDeliveryOpenOrder();
    vector<gateio::FutureAsset>& GetPerpetualAsset();
    vector<gateio::FuturePosition>& GetPerpetualPosition();
    vector<gateio::FutureOrder>& GetPerpetualOpenOrder();
    vector<gateio::CrossMarginAsset>& GetCrossMarginAsset();
    gateio::CrossMarginAccountTotal& GetCrossMarginAccountTotal();
    double GetDeliveryPositionValue(string asset);
    double GetPerpetualPositionValue(string asset);
    double GetDeliveryFloatAmount(string asset);
    double GetPerpetualFloatAmount(string asset);
    double GetPerpetualAssetTotal(string asset);
    void GetDeliveryLongShortFrozenPosition(string symbol, double longFrozenPos, double shortFrozenPos);
    void GetPerpetualLongShortFrozenPosition(string symbol, double longFrozenPos, double shortFrozenPos);
    int64_t GetUpdateTime();
    bool GetQueryStatus();
    vector<string>& GetQueryErrMsg();
    int isUnified();

private:
    AccountInfo accountInfo;
    GateioSpot* gateioSpot;
    GateioDelivery* gateioDelivery;
    GateioPerpetual* gateioPerpetual;
    GateioCrossMargin* gateioCrossMargin;
    bool spotEnable;
    bool deliveryEnable;
    bool perpetualEnable;
    bool crossMarginEnable;
    vector<gateio::SpotAsset> vSpotAsset;
    vector<gateio::FutureAsset> vDeliveryAsset;
    vector<gateio::FuturePosition> vDeliveryPosition;
    vector<gateio::FutureOrder> vDeliveryOpenOrder;
    vector<gateio::FutureAsset> vPerpetualAsset;
    vector<gateio::FuturePosition> vPerpetualPosition;
    vector<gateio::FutureOrder> vPerpetualOpenOrder;
    vector<gateio::CrossMarginAsset> vCrossMarginAsset;
    gateio::CrossMarginAccountTotal crossMarginAccountTotal;
    string baseAsset;
    int64_t updateTime;

    bool query;
    vector<string> vQueryErrMsg;
};
