#pragma once

#include "MonitorConfig.h"
#include "binance/BinanceSpot.h"
#include "binance/BinanceUFuture.h"
#include "binance/BinanceCFuture.h"
#include "binance/BinanceUnify.h"
#include "securitymanager.h"
#include <unordered_map>
#include <set>


class BinanceAdapterItem {
public:
    BinanceAdapterItem(AccountInfo info, sm::SecurityManager* s);
	~BinanceAdapterItem();
    void UpdateAccountInfo();   
    std::unordered_map<std::string, md::InstrumentInfo> GetInstrumentList();
    std::vector<binance::SpotAsset>& GetSpotAsset();
    std::vector<binance::UFutureAsset>& GetUFutureAsset();
    std::vector<binance::CFutureAsset>& GetCFutureAsset();
    std::vector<binance::UFuturePosition>& GetUFuturePosition();
    std::vector<binance::CFuturePosition>& GetCFuturePosition();
    std::vector<binance::PositionRisk>& GetUPositionRisk();
    std::vector<binance::PositionRisk>& GetCPositionRisk();
    double GetUPositionLiquidationPrice(std::string symbol, std::string positionSide);
    double GetCPositionLiquidationPrice(std::string symbol, std::string positionSide);
    std::vector<binance::SpotOpenOrder>& GetSpotOpenOrder();
    std::vector<binance::FutureOpenOrder>& GetUOpenOrder();
    std::vector<binance::FutureOpenOrder>& GetCOpenOrder();
    void GetULongShortFrozenPosition(std::string symbol, double& longFrozenPos, double& shortFrozenPos);
    void GetCLongShortFrozenPosition(std::string symbol, double& longFrozenPos, double& shortFrozenPos);
    double GetUAssetPositionValue(std::string asset);
    double GetCAssetPositionValue(std::string asset);
    double GetUFloatAmount(std::string asset);
    double GetCFloatAmount(std::string asset);
    int64_t GetUpdateTime();
    bool GetQueryStatus();
    std::vector<std::string>& GetQueryErrMsg();
    int isUnified();

    //unify
    std::vector<binance::UnifyAsset>& GetUnifyAsset();
    binance::UnifyAccount& GetUnifyAccount();
    std::vector<binance::UnifyPosition>& GetUmUnifyPosition();
    std::vector<binance::UnifyPosition>& GetCmUnifyPosition();
    std::vector<binance::UnifyOpenOrder>& GetUmUnifyOpenOrder();
    std::vector<binance::UnifyOpenOrder>& GetCmUnifyOpenOrder();
    double GetUnifyPositionValue(std::string asset);
    double GetUnifyFloatAmount(std::string asset);
    void GetUmUnifyLongShortFrozenPosition(std::string symbol, double& longFrozenPos, double& shortFrozenPos);
    void GetCmUnifyLongShortFrozenPosition(std::string symbol, double& longFrozenPos, double& shortFrozenPos);

private:
    AccountInfo accountInfo;

    std::unordered_map<std::stirng, md::InstrumentInfo> mInst;

    BinanceSpot* binanceSpot;
    BinanceUFuture* binanceUFuture;
    BinanceCFuture* binanceCFuture;
    BinanceUnify* binanceUnify;
    bool spotEnable;
    bool uFutureEnable;
    bool cFutureEnable;
    bool unifyEnable;

    std::vector<binance::SpotAsset> vSpotAsset;
    std::vector<binance::UFutureAsset> vUFutureAsset;
    std::vector<binance::CFutureAsset> vCFutureAsset;
    std::vector<binance::UFuturePosition> vUFuturePosition;
    std::vector<binance::CFuturePosition> vCFuturePosition;
    std::vector<binance::PositionRisk> vUPositionRisk;
    std::vector<binance::PositionRisk> vCPositionRisk;
    std::vector<binance::SpotOpenOrder> vSpotOpenOrder;
    std::vector<binance::FutureOpenOrder> vUOpenOrder;
    std::vector<binance::FutureOpenOrder> vCOpenOrder;

    // unify
    std::vector<binance::UnifyAsset> vUnifyAsset;
    binance::UnifyAccount unifyAccount;
    std::vector<binance::UnifyPosition> vUmUnifyPosition;
    std::vector<binance::UnifyPosition> vCmUnifyPosition;
    std::vector<binance::UnifyOpenOrder> vUmUnifyOpenOrder;
    std::vector<binance::UnifyOpenOrder> vCmUnifyOpenOrder;

    std::string baseAsset;
    int64_t updateTime;
    bool query;
    std::vector<std::string> vQueryErrMsg;

    sm::SecurityManager* smc;
};
