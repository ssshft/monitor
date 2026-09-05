#include "BinanceAdapterItem.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"

BinanceAdapterItem::BinanceAdapterItem(AccountInfo info) {
    accountInfo = info;
    binanceSpot = new BinanceSpot(accountInfo);
    binanceUFuture = new BinanceUFuture(accountInfo);
    binanceCFuture = new BinanceCFuture(accountInfo);
    binanceUnify = new BinanceUnify(accountInfo);

    baseAsset = accountInfo.baseAsset;
    updateTime = 0;
    query = true;
    spotEnable = false;
    uFutureEnable = false;
    cFutureEnable = false;
    savingEnable = false;
    marginEnable = false;
    loanEnable = false;
    unifyEnable = false;
    string apiPermission = accountInfo.apiPermission;
    vector<string> v;
    SplitString(apiPermission, ",", v);

    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == "all") {
            spotEnable = true;
            uFutureEnable = true;
            cFutureEnable = true;
            savingEnable = true;
            marginEnable = true;
            loanEnable = true;
        } else if (v[i] == "spot") {
            spotEnable = true;
        } else if (v[i] == "ufuture") {
            uFutureEnable = true;
        } else if (v[i] == "cfuture") {
            cFutureEnable = true;
        } else if (v[i] == "saving") {
            savingEnable = true;
        } else if (v[i] == "margin") {
            marginEnable = true;
        } else if (v[i] == "loan") {
            loanEnable = true;
        }
    }

    if (accountInfo.unified == 1) {
        if (v.size() == 0) {
            unifyEnable = true;
        } else {
            unifyEnable = false;
        }
    }
}

BinanceAdapterItem::~BinanceAdapterItem() {
    if (binanceSpot) {
        delete binanceSpot;
        binanceSpot = nullptr;
    }

    if (binanceUFuture) {
        delete binanceUFuture;
        binanceUFuture = nullptr;
    }

    if (binanceCFuture) {
        delete binanceCFuture;
        binanceCFuture = nullptr;
    }

    if (binanceSaving) {
        delete binanceSaving;
        binanceSaving = nullptr;
    }

    if (binanceMargin) {
        delete binanceMargin;
        binanceMargin = nullptr;
    }

    if (binanceLoan) {
        delete binanceLoan;
        binanceLoan = nullptr;
    }

    if (binanceUnify) {
        delete binanceUnify;
        binanceUnify = nullptr;
    }

    vSpotAsset.clear();
    vUFutureAsset.clear();
    vCFutureAsset.clear();
    vUFuturePosition.clear();
    vCFuturePosition.clear();
    vUPositionRisk.clear();
    vCPositionRisk.clear();
    vSpotOpenOrder.clear();
    vUOpenOrder.clear();
    vCOpenOrder.clear();
    vSavingAsset.clear();
    vLoanBorrow.clear();

    // unify
    vUnifyAsset.clear();
    vUmUnifyPosition.clear();
    vCmUnifyPosition.clear();
    vUmUnifyOpenOrder.clear();
    vCmUnifyOpenOrder.clear();
}

void BinanceAdapterItem::UpdateAccountInfo() {
    std::lock_guard<std::mutex> lk(mut);
    query = true;
    vQueryErrMsg.clear();

    if (savingEnable && binanceSaving) {
        vSavingAsset.clear();
        vector<string> vSavingErrMsg;
        bool savingQueryAccount = binanceSaving->QueryAccount(vSavingAsset, vSavingErrMsg);
        query = query && savingQueryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vSavingErrMsg.begin(), vSavingErrMsg.end());
    }

    if (spotEnable && binanceSpot) {
        vSpotAsset.clear();
        vector<string> vSpotErrMsg;
        bool spotQueryAccount = binanceSpot->QueryAccount(vSpotAsset, vSpotErrMsg);
        query = query && spotQueryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vSpotErrMsg.begin(), vSpotErrMsg.end());
    }

    if (uFutureEnable && binanceUFuture) {
        vUFutureAsset.clear();
        vUFuturePosition.clear();
        vUPositionRisk.clear();
        vUOpenOrder.clear();
        vector<string> vUFutureAccountErrMsg;
        bool uFutureQueryAccount = binanceUFuture->QueryAccount(vUFutureAsset, vUFuturePosition, vUFutureAccountErrMsg);
        query = query && uFutureQueryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vUFutureAccountErrMsg.begin(), vUFutureAccountErrMsg.end());

        vector<string> vUFuturePositionRiskErrMsg;
        bool uFutureQueryPositionRisk = binanceUFuture->QueryPositionRisk(vUPositionRisk, vUFuturePositionRiskErrMsg);
        query = query && uFutureQueryPositionRisk;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vUFuturePositionRiskErrMsg.begin(), vUFuturePositionRiskErrMsg.end());

        vector<string> vUFutureOpenOrderErrMsg;
        bool uFutureQueryOpenOrder = binanceUFuture->QueryOpenOrder(vUOpenOrder, vUFutureOpenOrderErrMsg);
        query = query && uFutureQueryOpenOrder;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vUFutureOpenOrderErrMsg.begin(), vUFutureOpenOrderErrMsg.end());
    }

    if (cFutureEnable && binanceCFuture) {
        vCFutureAsset.clear();
        vCFuturePosition.clear();
        vCPositionRisk.clear();
        vCOpenOrder.clear();
        vector<string> vCFutureErrMsg;
        bool cFutureQueryAccount = binanceCFuture->QueryAccount(vCFutureAsset, vCFuturePosition, vCFutureErrMsg);
        query = query && cFutureQueryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vCFutureErrMsg.begin(), vCFutureErrMsg.end());

        vector<string> vCFuturePositionRiskErrMsg;
        bool cFutureQueryPositionRisk = binanceCFuture->QueryPositionRisk(vCPositionRisk, vCFuturePositionRiskErrMsg);
        query = query && cFutureQueryPositionRisk;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vCFuturePositionRiskErrMsg.begin(), vCFuturePositionRiskErrMsg.end());

        vector<string> vCFutureOpenOrderErrMsg;
        bool cFutureQueryOpenOrder = binanceCFuture->QueryOpenOrder(vCOpenOrder, vCFutureOpenOrderErrMsg);
        query = query && cFutureQueryOpenOrder;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vCFutureOpenOrderErrMsg.begin(), vCFutureOpenOrderErrMsg.end());
    }

    if (marginEnable && binanceMargin) {
        vMarginAsset.clear();
        vector<string> vMarginErrMsg;
        bool marginQueryAccount = binanceMargin->QueryAccount(vMarginAsset, vMarginErrMsg);
        query = query && marginQueryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vMarginErrMsg.begin(), vMarginErrMsg.end());

        totalMarginAsset = binanceMargin->GetTotalAsset();
    }

    if (loanEnable && binanceLoan) {
        vLoanBorrow.clear();
        vector<string> vLoanErrMsg;
        bool loanQuery = binanceLoan->QueryLoanBorrow(vLoanBorrow, vLoanErrMsg);
        query = query && loanQuery;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vLoanErrMsg.begin(), vLoanErrMsg.end());
    }

    if (unifyEnable && binanceUnify) {
        vUnifyAsset.clear();
        vUmUnifyPosition.clear();
        vCmUnifyPosition.clear();
        vUmUnifyOpenOrder.clear();
        vCmUnifyOpenOrder.clear();

        vector<string> vBalanceErrMsg;
        binanceUnify->QueryBalance(vUnifyAsset, vBalanceErrMsg);
        vQueryErrMsg.insert(vQueryErrMsg.end(), vBalanceErrMsg.begin(), vBalanceErrMsg.end());

        vector<string> vAccountErrMsg;
        binanceUnify->QueryAccount(unifyAccount, vAccountErrMsg);
        vQueryErrMsg.insert(vQueryErrMsg.end(), vAccountErrMsg.begin(), vAccountErrMsg.end());

        vector<string> vUmPositionErrMsg;
        binanceUnify->QueryUmPosition(vUmUnifyPosition, vUmPositionErrMsg);
        vQueryErrMsg.insert(vQueryErrMsg.end(), vUmPositionErrMsg.begin(), vUmPositionErrMsg.end());

        vector<string> vCmPositionErrMsg;
        binanceUnify->QueryCmPosition(vCmUnifyPosition, vCmPositionErrMsg);
        vQueryErrMsg.insert(vQueryErrMsg.end(), vCmPositionErrMsg.begin(), vCmPositionErrMsg.end());

        vector<string> vUmOpenOrderErrMsg;
	    binanceUnify->QueryUmOpenOrder(vUmUnifyOpenOrder, vUmOpenOrderErrMsg);
        vQueryErrMsg.insert(vQueryErrMsg.end(), vUmOpenOrderErrMsg.begin(), vUmOpenOrderErrMsg.end());

        vector<string> vCmOpenOrderErrMsg;
        binanceUnify->QueryCmOpenOrder(vCmUnifyOpenOrder, vCmOpenOrderErrMsg);
        vQueryErrMsg.insert(vQueryErrMsg.end(), vCmOpenOrderErrMsg.begin(), vCmOpenOrderErrMsg.end());
    }

    updateTime = GetCurrentTimeUs();
}

set<string> BinanceAdapterItem::GetInstrumentList() {
    set<string> s;
    if (baseAsset != "USDT") {
        string instrumentKey = "BINANCE|" + baseAsset + "USDT" + "|SPOT";
        s.insert(instrumentKey);
    }

    for (size_t i = 0; i < vSpotAsset.size(); ++i) {
        string asset = vSpotAsset[i].assetType;
        if (asset != baseAsset && asset != "USDT") {
            string instrumentKey = "BINANCE|" + asset + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "BINANCE|" + asset + "USDT" + "|SPOT";
            s.insert(instrumentKey);
        }
    }

    for (size_t i = 0; i < vUFutureAsset.size(); ++i) {
        string asset = vUFutureAsset[i].assetType;
        if (asset != baseAsset && asset != "USDT") {
            string instrumentKey = "BINANCE|" + asset + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "BINANCE|" + asset + "USDT" + "|SPOT";
            s.insert(instrumentKey);
        }
    }
    
    for (size_t i = 0; i < vCFutureAsset.size(); ++i) {
        string asset = vCFutureAsset[i].assetType;
        if (asset != baseAsset && asset != "USDT") {
            string instrumentKey = "BINANCE|" + asset + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "BINANCE|" + asset + "USDT" + "|SPOT";
            s.insert(instrumentKey);
        }
    }

    for (size_t i = 0; i < vUFuturePosition.size(); ++i) {
        string instrumentKey = "BINANCE|" + vUFuturePosition[i].symbol + "|FUTURES";
        s.insert(instrumentKey);

        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        instrumentKey = "BINANCE|" + info.instLeft + baseAsset + "|SPOT";
        s.insert(instrumentKey);
        instrumentKey = "BINANCE|" + info.instLeft + "USDT" + "|SPOT";
        s.insert(instrumentKey);
    }
    
    for (size_t i = 0; i < vCFuturePosition.size(); ++i) {
        string instrumentKey = "BINANCE|" + vCFuturePosition[i].symbol + "|FUTURES";
        s.insert(instrumentKey);
    }

    return s;
}

vector<binance::SpotAsset>& BinanceAdapterItem::GetSpotAsset() {
    return vSpotAsset;
}

vector<binance::UFutureAsset>& BinanceAdapterItem::GetUFutureAsset() {
    return vUFutureAsset;
}

vector<binance::CFutureAsset>& BinanceAdapterItem::GetCFutureAsset() {
    return vCFutureAsset;
}

vector<binance::UFuturePosition>& BinanceAdapterItem::GetUFuturePosition() {
    return vUFuturePosition;
}

vector<binance::CFuturePosition>& BinanceAdapterItem::GetCFuturePosition() {
    return vCFuturePosition;
}

vector<binance::PositionRisk>& BinanceAdapterItem::GetUPositionRisk() {
    return vUPositionRisk;
}

vector<binance::PositionRisk>& BinanceAdapterItem::GetCPositionRisk() {
    return vCPositionRisk;
}

double BinanceAdapterItem::GetUPositionLiquidationPrice(string symbol, string positionSide) {
    double liquidationPrice = 0.0;
    for (size_t i = 0; i < vUPositionRisk.size(); ++i) {
        if (vUPositionRisk[i].symbol == symbol && vUPositionRisk[i].positionSide == positionSide) {
            liquidationPrice = vUPositionRisk[i].liquidationPrice;
        }
    }
    return liquidationPrice;
}

double BinanceAdapterItem::GetCPositionLiquidationPrice(string symbol, string positionSide) {
    double liquidationPrice = 0.0;
    for (size_t i = 0; i < vCPositionRisk.size(); ++i) {
        if (vCPositionRisk[i].symbol == symbol && vCPositionRisk[i].positionSide == positionSide) {
            liquidationPrice = vCPositionRisk[i].liquidationPrice;
        }
    }
    return liquidationPrice;
}

vector<binance::SpotOpenOrder>& BinanceAdapterItem::GetSpotOpenOrder() {
    return vSpotOpenOrder;
}

vector<binance::FutureOpenOrder>& BinanceAdapterItem::GetUOpenOrder() {
    return vUOpenOrder;
}

vector<binance::FutureOpenOrder>& BinanceAdapterItem::GetCOpenOrder() {
    return vCOpenOrder;
}

void BinanceAdapterItem::GetULongShortFrozenPosition(string symbol, double& longFrozenPos, double& shortFrozenPos) {
    for (size_t i = 0; i < vUOpenOrder.size(); ++i) {
        if (vUOpenOrder[i].symbol == symbol) {
            if (vUOpenOrder[i].side == "BUY") {
                longFrozenPos += vUOpenOrder[i].origQty - vUOpenOrder[i].executedQty;
            } else {
                shortFrozenPos += vUOpenOrder[i].origQty - vUOpenOrder[i].executedQty;
            }
        }
    }
}

void BinanceAdapterItem::GetCLongShortFrozenPosition(string symbol, double& longFrozenPos, double& shortFrozenPos) {
    for (size_t i = 0; i < vCOpenOrder.size(); ++i) {
        if (vCOpenOrder[i].symbol == symbol) {
            if (vCOpenOrder[i].side == "BUY") {
                longFrozenPos += vCOpenOrder[i].origQty - vCOpenOrder[i].executedQty;
            } else {
                shortFrozenPos += vCOpenOrder[i].origQty - vCOpenOrder[i].executedQty;
            }
        }
    }
}

double BinanceAdapterItem::GetUAssetPositionValue(string asset) {
    double positionValue = 0.0;
    for (size_t i = 0; i < vUFuturePosition.size(); ++i) {
        string instrumentKey = "BINANCE|" + vUFuturePosition[i].symbol + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0) {
            double posValue = vUFuturePosition[i].positionAmt * price * info.multiple;
            positionValue += fabs(posValue);
            LOG_INFO("GetUAssetPositionValue: accountId:%d instrumentKey:%s asset:%s positionValueD:%f", accountInfo.accountId, instrumentKey.c_str(), asset.c_str(), posValue); 
        } else {
            LOG_INFO("GetUAssetPositionValue positionValue=0: accountId:%d instrumentKey:%s asset:%s price:%f key:%s info.margin:%s", accountInfo.accountId, instrumentKey.c_str(), asset.c_str(), price, key.c_str(), info.margin.c_str()); 
        }
    }
    return positionValue;
}

double BinanceAdapterItem::GetCAssetPositionValue(string asset) {
    double positionValue = 0.0;
    for (size_t i = 0; i < vCFuturePosition.size(); ++i) {
        string instrumentKey = "BINANCE|" + vCFuturePosition[i].symbol + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0) {
            positionValue += fabs(vCFuturePosition[i].positionAmt / price * info.multiple);
        }
    }
    return positionValue;
}

double BinanceAdapterItem::GetUFloatAmount(string asset) {
    double floatAmount = 0.0;
    for (size_t i = 0; i < vUFuturePosition.size(); ++i) {
        string instrumentKey = "BINANCE|" + vUFuturePosition[i].symbol + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0) {
            floatAmount += (price - vUFuturePosition[i].entryPrice) * vUFuturePosition[i].positionAmt * info.multiple;

            stringstream ss;
            ss << "accountId:" << accountInfo.accountId << " instrumentKey:" << instrumentKey << " price:" << price << " entryPrice:" << vUFuturePosition[i].entryPrice << " positionAmt:" << vUFuturePosition[i].positionAmt << " multiple:" << info.multiple << " floatAmount:" << floatAmount;
            LOG_INFO("GetUFloatAmount: %s", ss.str().c_str()); 
        }
    }
    return floatAmount;
}

double BinanceAdapterItem::GetCFloatAmount(string asset) {
    double floatAmount = 0.0;
    for (size_t i = 0; i < vCFuturePosition.size(); ++i) {
        string instrumentKey = "BINANCE|" + vCFuturePosition[i].symbol + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0 && vCFuturePosition[i].entryPrice > 0.0) {
            floatAmount += (1 / vCFuturePosition[i].entryPrice - 1 / price) * vCFuturePosition[i].positionAmt * info.multiple;

            stringstream ss;
            ss << "accountId:" << accountInfo.accountId << " instrumentKey:" << instrumentKey << " price:" << price << " entryPrice:" << vCFuturePosition[i].entryPrice << " positionAmt:" << vCFuturePosition[i].positionAmt << " multiple:" << info.multiple << " floatAmount:" << floatAmount;
            LOG_INFO("GetCFloatAmount: %s", ss.str().c_str()); 
        }
    }
    return floatAmount;
}

vector<binance::SavingAsset>& BinanceAdapterItem::GetSavingAsset() {
    return vSavingAsset;
}

vector<binance::MarginAsset>& BinanceAdapterItem::GetMarginAsset() {
    return vMarginAsset;
}

vector<binance::LoanBorrow>& BinanceAdapterItem::GetLoanBorrow() {
    return vLoanBorrow;
}

binance::TotalMarginAsset& BinanceAdapterItem::GetTotalMarginAsset() {
    return totalMarginAsset;
}

int64_t BinanceAdapterItem::GetUpdateTime() {
    return updateTime;
}

bool BinanceAdapterItem::GetQueryStatus() {
    return query;
}

vector<string>& BinanceAdapterItem::GetQueryErrMsg() {
    return vQueryErrMsg;
}

int BinanceAdapterItem::isUnified() {
    return accountInfo.unified;
}

vector<binance::UnifyAsset>& BinanceAdapterItem::GetUnifyAsset() {
    return vUnifyAsset;
}

binance::UnifyAccount& BinanceAdapterItem::GetUnifyAccount() {
    return unifyAccount;
}

vector<binance::UnifyPosition>& BinanceAdapterItem::GetUmUnifyPosition() {
    return vUmUnifyPosition;
}

vector<binance::UnifyPosition>& BinanceAdapterItem::GetCmUnifyPosition() {
    return vCmUnifyPosition;
}

vector<binance::UnifyOpenOrder>& BinanceAdapterItem::GetUmUnifyOpenOrder() {
    return vUmUnifyOpenOrder;
}

vector<binance::UnifyOpenOrder>& BinanceAdapterItem::GetCmUnifyOpenOrder() {
    return vCmUnifyOpenOrder;
}

double BinanceAdapterItem::GetUnifyPositionValue(string asset) {
    double positionValue = 0.0;
    for (size_t i = 0; i < vUmUnifyPosition.size(); ++i) {
        string instrumentKey = "BINANCE|" + vUmUnifyPosition[i].symbol + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0) {
            double posValue = vUmUnifyPosition[i].positionAmt * price * info.multiple;
            positionValue += fabs(posValue);
            LOG_INFO("GetUnifyPositionValue: accountId:%d instrumentKey:%s asset:%s positionValueD:%f", accountInfo.accountId, instrumentKey.c_str(), asset.c_str(), posValue); 
        } else {
            LOG_INFO("GetUnifyPositionValue positionValue=0: accountId:%d instrumentKey:%s asset:%s price:%f key:%s info.margin:%s", accountInfo.accountId, instrumentKey.c_str(), asset.c_str(), price, key.c_str(), info.margin.c_str()); 
        }
    }

    for (size_t i = 0; i < vCmUnifyPosition.size(); ++i) {
        string instrumentKey = "BINANCE|" + vCmUnifyPosition[i].symbol + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0) {
            positionValue += fabs(vCmUnifyPosition[i].positionAmt / price * info.multiple);
        }
    }
    return positionValue;
}

double BinanceAdapterItem::GetUnifyFloatAmount(string asset) {
    double floatAmount = 0.0;
    for (size_t i = 0; i < vUmUnifyPosition.size(); ++i) {
        string instrumentKey = "BINANCE|" + vUmUnifyPosition[i].symbol + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0) {
            floatAmount += (price - vUmUnifyPosition[i].entryPrice) * vUmUnifyPosition[i].positionAmt * info.multiple;

            stringstream ss;
            ss << "accountId:" << accountInfo.accountId << " instrumentKey:" << instrumentKey << " price:" << price << " entryPrice:" << vUmUnifyPosition[i].entryPrice << " positionAmt:" << vUmUnifyPosition[i].positionAmt << " multiple:" << info.multiple << " floatAmount:" << floatAmount;
            LOG_INFO("GetUFloatAmount: %s", ss.str().c_str()); 
        }
    }

    for (size_t i = 0; i < vCmUnifyPosition.size(); ++i) {
        string instrumentKey = "BINANCE|" + vCmUnifyPosition[i].symbol + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0 && vCmUnifyPosition[i].entryPrice > 0.0) {
            floatAmount += (1 / vCmUnifyPosition[i].entryPrice - 1 / price) * vCmUnifyPosition[i].positionAmt * info.multiple;

            stringstream ss;
            ss << "accountId:" << accountInfo.accountId << " instrumentKey:" << instrumentKey << " price:" << price << " entryPrice:" << vCmUnifyPosition[i].entryPrice << " positionAmt:" << vCmUnifyPosition[i].positionAmt << " multiple:" << info.multiple << " floatAmount:" << floatAmount;
            LOG_INFO("GetCFloatAmount: %s", ss.str().c_str()); 
        }
    }
    return floatAmount;
}

void BinanceAdapterItem::GetUmUnifyLongShortFrozenPosition(string symbol, double& longFrozenPos, double& shortFrozenPos) {
    for (size_t i = 0; i < vUmUnifyOpenOrder.size(); ++i) {
        if (vUmUnifyOpenOrder[i].symbol == symbol) {
            if (vUmUnifyOpenOrder[i].side == "BUY") {
                longFrozenPos += vUmUnifyOpenOrder[i].origQty - vUmUnifyOpenOrder[i].executedQty;
            } else {
                shortFrozenPos += vUmUnifyOpenOrder[i].origQty - vUmUnifyOpenOrder[i].executedQty;
            }
        }
    }
}

void BinanceAdapterItem::GetCmUnifyLongShortFrozenPosition(string symbol, double& longFrozenPos, double& shortFrozenPos) {
    for (size_t i = 0; i < vCmUnifyOpenOrder.size(); ++i) {
        if (vCmUnifyOpenOrder[i].symbol == symbol) {
            if (vCmUnifyOpenOrder[i].side == "BUY") {
                longFrozenPos += vCmUnifyOpenOrder[i].origQty - vCmUnifyOpenOrder[i].executedQty;
            } else {
                shortFrozenPos += vCmUnifyOpenOrder[i].origQty - vCmUnifyOpenOrder[i].executedQty;
            }
        }
    }
}