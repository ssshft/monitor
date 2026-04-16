#pragma once

#include "MonitorConfig.h"
#include "binance/BinanceSpot.h"
#include "binance/BinanceUFuture.h"
#include "binance/BinanceCFuture.h"
#include "binance/BinanceSaving.h"
#include "binance/BinanceMargin.h"
#include "binance/BinanceLoan.h"
#include "binance/BinanceUnify.h"
#include <map>
#include <set>

using namespace std;

class BinanceAdapterItem {
public:
    BinanceAdapterItem(AccountInfo info);
	~BinanceAdapterItem();
    void UpdateAccountInfo();   
    set<string> GetInstrumentList();
    vector<binance::SpotAsset>& GetSpotAsset();
    vector<binance::UFutureAsset>& GetUFutureAsset();
    vector<binance::CFutureAsset>& GetCFutureAsset();
    vector<binance::UFuturePosition>& GetUFuturePosition();
    vector<binance::CFuturePosition>& GetCFuturePosition();
    vector<binance::PositionRisk>& GetUPositionRisk();
    vector<binance::PositionRisk>& GetCPositionRisk();
    double GetUPositionLiquidationPrice(string symbol, string positionSide);
    double GetCPositionLiquidationPrice(string symbol, string positionSide);
    vector<binance::SpotOpenOrder>& GetSpotOpenOrder();
    vector<binance::FutureOpenOrder>& GetUOpenOrder();
    vector<binance::FutureOpenOrder>& GetCOpenOrder();
    void GetULongShortFrozenPosition(string symbol, double& longFrozenPos, double& shortFrozenPos);
    void GetCLongShortFrozenPosition(string symbol, double& longFrozenPos, double& shortFrozenPos);
    double GetUAssetPositionValue(string asset);
    double GetCAssetPositionValue(string asset);
    double GetUFloatAmount(string asset);
    double GetCFloatAmount(string asset);
    vector<binance::SavingAsset>& GetSavingAsset();
    vector<binance::MarginAsset>& GetMarginAsset();
    vector<binance::LoanBorrow>& GetLoanBorrow();
    binance::TotalMarginAsset& GetTotalMarginAsset();
    int64_t GetUpdateTime();
    bool GetQueryStatus();
    vector<string>& GetQueryErrMsg();
    int isUnified();

    //unify
    vector<binance::UnifyAsset>& GetUnifyAsset();
    binance::UnifyAccount& GetUnifyAccount();
    vector<binance::UnifyPosition>& GetUmUnifyPosition();
    vector<binance::UnifyPosition>& GetCmUnifyPosition();
    vector<binance::UnifyOpenOrder>& GetUmUnifyOpenOrder();
    vector<binance::UnifyOpenOrder>& GetCmUnifyOpenOrder();
    double GetUnifyPositionValue(string asset);
    double GetUnifyFloatAmount(string asset);
    void GetUmUnifyLongShortFrozenPosition(string symbol, double& longFrozenPos, double& shortFrozenPos);
    void GetCmUnifyLongShortFrozenPosition(string symbol, double& longFrozenPos, double& shortFrozenPos);

private:
    AccountInfo accountInfo;

    BinanceSpot* binanceSpot;
    BinanceUFuture* binanceUFuture;
    BinanceCFuture* binanceCFuture;
    BinanceSaving* binanceSaving;
    BinanceMargin* binanceMargin;
    BinanceLoan* binanceLoan;
    BinanceUnify* binanceUnify;
    bool spotEnable;
    bool uFutureEnable;
    bool cFutureEnable;
    bool savingEnable;
    bool marginEnable;
    bool loanEnable;
    bool unifyEnable;

    vector<binance::SpotAsset> vSpotAsset;
    vector<binance::UFutureAsset> vUFutureAsset;
    vector<binance::CFutureAsset> vCFutureAsset;
    vector<binance::UFuturePosition> vUFuturePosition;
    vector<binance::CFuturePosition> vCFuturePosition;
    vector<binance::PositionRisk> vUPositionRisk;
    vector<binance::PositionRisk> vCPositionRisk;
    vector<binance::SpotOpenOrder> vSpotOpenOrder;
    vector<binance::FutureOpenOrder> vUOpenOrder;
    vector<binance::FutureOpenOrder> vCOpenOrder;
    vector<binance::SavingAsset> vSavingAsset;
    vector<binance::MarginAsset> vMarginAsset;
    vector<binance::LoanBorrow> vLoanBorrow;
    binance::TotalMarginAsset totalMarginAsset;

    // unify
    vector<binance::UnifyAsset> vUnifyAsset;
    binance::UnifyAccount unifyAccount;
    vector<binance::UnifyPosition> vUmUnifyPosition;
    vector<binance::UnifyPosition> vCmUnifyPosition;
    vector<binance::UnifyOpenOrder> vUmUnifyOpenOrder;
    vector<binance::UnifyOpenOrder> vCmUnifyOpenOrder;

    string baseAsset;
    int64_t updateTime;
    bool query;
    vector<string> vQueryErrMsg;
};
