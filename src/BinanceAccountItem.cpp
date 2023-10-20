#include "BinanceAccountItem.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"
#include "BinanceAdapterMgr.h"
#include "BinanceSystemMgr.h"
#include "CoinbaseAdapterMgr.h"
#include "CoinbaseMdMgr.h"
#include "GateioAdapterMgr.h"
#include "BybitAdapterMgr.h"


BinanceAccountItem::BinanceAccountItem(int id, string n, string ty, string ex, int he) {
    customerId = id;
    type = ty;
    name = n;
    type = ty;
    exchangeStr = ex;
    hedge = he;
    unified = 0;
    baseAsset = MonitorConfig::GetInstance().GetBaseAssetById(customerId);
    riskInfo.accountId = customerId;
    riskInfo.name = name;
    riskInfo.baseAsset = baseAsset;
    MINDOUBLE = 0.0000000001;
    adapterQuery = true;

    alarmInfo = MonitorConfig::GetInstance().GetAlarmInfoById(customerId);
    triggerInterval = MonitorConfig::GetInstance().GetTriggerInterval();

    maxLeverageUD = true;
    maxLeverageUS = true;
    maxLeverageTabD = "";
    maxLeverageTabS = "";
    maxRiskExposureTab = "Exposure";

    subMsgFlag = false;

    totalMarginBalance = 0.0;
    initialMarginRate = 0.0;
    unifyMaintenanceMarginRate = 0.0;

    totalExposure = 0.0;
}

BinanceAccountItem::~BinanceAccountItem() {
    Clear();
}

void BinanceAccountItem::UpdateBySystem() {
    BinanceSystemItem* item = BinanceSystemMgr::GetInstance().GetSystemItem(customerId);
    if (item) {
        riskInfo.sUpdateTime = item->GetUpdateTime();
        string exchangeStr = item->GetExchangeStr();

        // spot
        vector<igsystem::SysAsset>& vSysSpotAsset = item->GetSpotAsset();
        for (size_t i = 0; i < vSysSpotAsset.size(); ++i) {
            string asset = vSysSpotAsset[i].asset;
            auto iter = mSpotAsset.find(asset);
            if (iter == mSpotAsset.end()) {
                igmonitor::Asset asset;
                asset.asset = vSysSpotAsset[i].asset;
                asset.transferAmount = vSysSpotAsset[i].totalInoutAmount;
                asset.transferFrozenAmount = vSysSpotAsset[i].transFrozenAmount;
                double totalAmount = vSysSpotAsset[i].totalAmount;
                double frozenAmount = vSysSpotAsset[i].frozenAmount;
                asset.netAmountS = totalAmount;
                asset.availableAmountS = totalAmount - frozenAmount;
                asset.totalAmountS = totalAmount;
                asset.frozenAmountS = frozenAmount;
                asset.initialAmount = vSysSpotAsset[i].initAmount;
                mSpotAsset.insert(make_pair(asset.asset, asset));
            } else {
                iter->second.transferAmount = vSysSpotAsset[i].totalInoutAmount;
                iter->second.transferFrozenAmount = vSysSpotAsset[i].transFrozenAmount;
                double totalAmount = vSysSpotAsset[i].totalAmount;
                double frozenAmount = vSysSpotAsset[i].frozenAmount;
                iter->second.netAmountS = totalAmount;
                iter->second.availableAmountS = totalAmount - frozenAmount;
                iter->second.totalAmountS = totalAmount;
                iter->second.frozenAmountS = frozenAmount;
                iter->second.initialAmount = vSysSpotAsset[i].initAmount;
            }
        }

        // ufuture
        vector<igsystem::SysAsset>& vSysUFutureAsset = item->GetUFutureAsset();
        for (size_t i = 0; i < vSysUFutureAsset.size(); ++i) {
            string asset = vSysUFutureAsset[i].asset;
            auto iter = mUFutureAsset.find(asset);
            if (iter == mUFutureAsset.end()) {
                igmonitor::Asset asset;
                asset.asset = vSysUFutureAsset[i].asset;
                asset.positionValueS = vSysUFutureAsset[i].positionValue;
                asset.transferAmount = vSysUFutureAsset[i].totalInoutAmount;
                asset.transferFrozenAmount = vSysUFutureAsset[i].transFrozenAmount;
                double totalAmount = vSysUFutureAsset[i].totalAmount;
                double floatAmount = vSysUFutureAsset[i].floatAmount;
                double frozenAmount = vSysUFutureAsset[i].frozenAmount;
                asset.netAmountS = totalAmount + floatAmount;
                asset.availableAmountS = totalAmount - frozenAmount;
                asset.totalAmountS = totalAmount;
                asset.floatAmountS = floatAmount;
                asset.marginAmountS = vSysUFutureAsset[i].marginAmount;
                asset.frozenMarginAmountS = vSysUFutureAsset[i].frozenMargin;
                asset.initialAmount = vSysUFutureAsset[i].initAmount;
                if (fabs(asset.netAmountS) > MINDOUBLE) {
                    asset.realLeverageRatioS = fabs(asset.positionValueS / asset.netAmountS);
                }
                mUFutureAsset.insert(make_pair(asset.asset, asset));
            } else {
                iter->second.positionValueS = vSysUFutureAsset[i].positionValue;
                iter->second.transferAmount = vSysUFutureAsset[i].totalInoutAmount;
                iter->second.transferFrozenAmount = vSysUFutureAsset[i].transFrozenAmount;
                double totalAmount = vSysUFutureAsset[i].totalAmount;
                double floatAmount = vSysUFutureAsset[i].floatAmount;
                double frozenAmount = vSysUFutureAsset[i].frozenAmount;
                iter->second.netAmountS = totalAmount + floatAmount;
                iter->second.availableAmountS = totalAmount - frozenAmount;
                iter->second.totalAmountS = totalAmount;
                iter->second.floatAmountS = floatAmount;
                iter->second.marginAmountS = vSysUFutureAsset[i].marginAmount;
                iter->second.frozenMarginAmountS = vSysUFutureAsset[i].frozenMargin;
                iter->second.initialAmount = vSysUFutureAsset[i].initAmount;
                if (fabs(iter->second.netAmountS) > MINDOUBLE) {
                    iter->second.realLeverageRatioS = fabs(iter->second.positionValueS / iter->second.netAmountS);
                }
            }
        }

        vector<igsystem::SysPosition>& vSysUFuturePosition = item->GetUFuturePosition();
        for (size_t i = 0; i < vSysUFuturePosition.size(); ++i) {
            string symbol = vSysUFuturePosition[i].symbol;
            auto iter = mUFuturePosition.find(symbol);
            if (iter == mUFuturePosition.end()) {
                igmonitor::Position position;
                position.symbol = vSysUFuturePosition[i].symbol;
                position.netPositionS = vSysUFuturePosition[i].longPosition - vSysUFuturePosition[i].shortPosition;
                double longAvgPrice = vSysUFuturePosition[i].longAvgPrice;
                double shortAvgPrice = vSysUFuturePosition[i].shortAvgPrice;
                position.netAvgPriceS = shortAvgPrice != -1 ? shortAvgPrice : longAvgPrice;
                position.floatAmountS = vSysUFuturePosition[i].floatAmount;
                position.underwayNetPositionS = vSysUFuturePosition[i].frozenLongPosition - vSysUFuturePosition[i].frozenShortPosition;
                position.underwayAbsPositionS = fabs(position.underwayNetPositionS);
                string instKey = exchangeStr + "|" + position.symbol + "|FUTURES";
                position.key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
                mUFuturePosition.insert(make_pair(position.symbol, position));
            } else {
                iter->second.netPositionS = vSysUFuturePosition[i].longPosition - vSysUFuturePosition[i].shortPosition;
                double longAvgPrice = vSysUFuturePosition[i].longAvgPrice;
                double shortAvgPrice = vSysUFuturePosition[i].shortAvgPrice;
                iter->second.netAvgPriceS = shortAvgPrice != -1 ? shortAvgPrice : longAvgPrice;
                iter->second.floatAmountS = vSysUFuturePosition[i].floatAmount;
                iter->second.underwayNetPositionS = vSysUFuturePosition[i].frozenLongPosition - vSysUFuturePosition[i].frozenShortPosition;
                iter->second.underwayAbsPositionS = fabs(iter->second.underwayNetPositionS);
            }
        }

        // cfuture
        vector<igsystem::SysAsset>& vSysCFutureAsset = item->GetCFutureAsset();
        for (size_t i = 0; i < vSysCFutureAsset.size(); ++i) {
            string asset = vSysCFutureAsset[i].asset;
            auto iter = mCFutureAsset.find(asset);
            if (iter == mCFutureAsset.end()) {
                igmonitor::Asset asset;
                asset.asset = vSysCFutureAsset[i].asset;
                asset.positionValueS = vSysCFutureAsset[i].positionValue;
                asset.transferAmount = vSysCFutureAsset[i].totalInoutAmount;
                asset.transferFrozenAmount = vSysCFutureAsset[i].transFrozenAmount;
                double totalAmount = vSysCFutureAsset[i].totalAmount;
                double floatAmount = vSysCFutureAsset[i].floatAmount;
                double frozenAmount = vSysCFutureAsset[i].frozenAmount;
                asset.netAmountS = totalAmount + floatAmount;
                asset.availableAmountS = totalAmount - frozenAmount;
                asset.totalAmountS = totalAmount;
                asset.floatAmountS = floatAmount;
                asset.marginAmountS = vSysCFutureAsset[i].marginAmount;
                asset.frozenMarginAmountS = vSysCFutureAsset[i].frozenMargin;
                asset.initialAmount = vSysCFutureAsset[i].initAmount;
                if (fabs(asset.netAmountS) > MINDOUBLE) {
                    asset.realLeverageRatioS = fabs(asset.positionValueS / asset.netAmountS);
                }
                mCFutureAsset.insert(make_pair(asset.asset, asset));
            } else {
                iter->second.positionValueS = vSysCFutureAsset[i].positionValue;
                iter->second.transferAmount = vSysCFutureAsset[i].totalInoutAmount;
                iter->second.transferFrozenAmount = vSysCFutureAsset[i].transFrozenAmount;
                double totalAmount = vSysCFutureAsset[i].totalAmount;
                double floatAmount = vSysCFutureAsset[i].floatAmount;
                double frozenAmount = vSysCFutureAsset[i].frozenAmount;
                iter->second.netAmountS = totalAmount + floatAmount;
                iter->second.availableAmountS = totalAmount - frozenAmount;
                iter->second.totalAmountS = totalAmount;
                iter->second.floatAmountS = floatAmount;
                iter->second.marginAmountS = vSysCFutureAsset[i].marginAmount;
                iter->second.frozenMarginAmountS = vSysCFutureAsset[i].frozenMargin;
                iter->second.initialAmount = vSysCFutureAsset[i].initAmount;
                if (fabs(iter->second.netAmountS) > MINDOUBLE) {
                    iter->second.realLeverageRatioS = fabs(iter->second.positionValueS / iter->second.netAmountS);
                }
            }
        }

        vector<igsystem::SysPosition>& vSysCFuturePosition = item->GetCFuturePosition();
        for (size_t i = 0; i < vSysCFuturePosition.size(); ++i) {
            string symbol = vSysCFuturePosition[i].symbol;
            auto iter = mCFuturePosition.find(symbol);
            if (iter == mCFuturePosition.end()) {
                igmonitor::Position position;
                position.symbol = vSysCFuturePosition[i].symbol;
                position.netPositionS = vSysCFuturePosition[i].longPosition - vSysCFuturePosition[i].shortPosition;
                double longAvgPrice = vSysCFuturePosition[i].longAvgPrice;
                double shortAvgPrice = vSysCFuturePosition[i].shortAvgPrice;
                position.netAvgPriceS = position.netPositionS < 0 ? shortAvgPrice : longAvgPrice;
                position.floatAmountS = vSysCFuturePosition[i].floatAmount;
                position.underwayNetPositionS = vSysCFuturePosition[i].frozenLongPosition - vSysCFuturePosition[i].frozenShortPosition;
                position.underwayAbsPositionS = fabs(position.underwayNetPositionS);
                string instKey = exchangeStr + "|" + position.symbol + "|FUTURES";
                position.key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
                mCFuturePosition.insert(make_pair(position.symbol, position));
            } else {
                iter->second.netPositionS = vSysCFuturePosition[i].longPosition - vSysCFuturePosition[i].shortPosition;
                double longAvgPrice = vSysCFuturePosition[i].longAvgPrice;
                double shortAvgPrice = vSysCFuturePosition[i].shortAvgPrice;
                iter->second.netAvgPriceS = iter->second.netPositionS < 0 ? shortAvgPrice : longAvgPrice;
                iter->second.floatAmountS = vSysCFuturePosition[i].floatAmount;
                iter->second.underwayNetPositionS = vSysCFuturePosition[i].frozenLongPosition - vSysCFuturePosition[i].frozenShortPosition;
                iter->second.underwayAbsPositionS = fabs(iter->second.underwayNetPositionS);
            }
        }
    }
}

void BinanceAccountItem::UpdateByAdapter() {
    if (exchangeStr == "BINANCE") {
        UpdateByBinanceAdapter();
    } else if (exchangeStr == "COINBASE") {
        UpdateByCoinbaseAdapter();
    } else if (exchangeStr == "GATEIO") {
        UpdateByGateioAdapter();
    } else if (exchangeStr == "BYBIT") {
        UpdateByBybitAdapter();
    }
}

void BinanceAccountItem::UpdateByBinanceAdapter() {
    BinanceAdapterItem* item = BinanceAdapterMgr::GetInstance().GetAdapterItem(customerId);
    if (item) {
        riskInfo.dUpdateTime = item->GetUpdateTime();
        adapterQuery = item->GetQueryStatus();
        adapterQueryErrMsg = item->GetQueryErrMsg();
        unified = item->isUnified();

        binance::UnifyAccount& unifyAccount = item->GetUnifyAccount();
        totalMarginBalance = unifyAccount.accountEquity;
        unifyMaintenanceMarginRate = unifyAccount.uniMMR;

        // saving
        vector<binance::SavingAsset>& vSavingAsset = item->GetSavingAsset();
        for (size_t i = 0; i < vSavingAsset.size(); ++i) {
            igmonitor::SavAsset savAsset;
            savAsset.asset = vSavingAsset[i].asset;
            savAsset.amountD = vSavingAsset[i].amount;
            savAsset.amountInBTCD = vSavingAsset[i].amountInBTC;
            savAsset.amountInUSDTD = vSavingAsset[i].amountInUSDT;
            mSavAsset.insert(make_pair(savAsset.asset, savAsset));
        }


        // spot
        vector<binance::SpotAsset>& vSpotAsset = item->GetSpotAsset();
        for (size_t i = 0; i < vSpotAsset.size(); ++i) {
            string ass = vSpotAsset[i].assetType;
            bool exist = false;
            if (ass.substr(0, 2) == "LD") {
                string savingAssetType = ass.substr(2, ass.size());
                auto it = mSavAsset.find(savingAssetType);
                if (it != mSavAsset.end() && it->second.amountD >= MINDOUBLE) {
                    exist = true;
                }
            }

            if (!exist) {
                auto iter = mSpotAsset.find(ass);
                if (iter == mSpotAsset.end()) {
                    igmonitor::Asset asset;
                    asset.asset = vSpotAsset[i].assetType;
                    asset.frozenAmountD = vSpotAsset[i].locked;
                    asset.totalAmountD = vSpotAsset[i].free + vSpotAsset[i].locked;
                    asset.netAmountD = asset.totalAmountD;
                    asset.availableAmountD = vSpotAsset[i].free;
                    asset.underwayOrderValueD = GetUnderwayOrderValue(asset.asset, asset.frozenAmountD);
                    mSpotAsset.insert(make_pair(asset.asset, asset));
                } else {
                    iter->second.frozenAmountD = vSpotAsset[i].locked;
                    iter->second.totalAmountD = vSpotAsset[i].free + vSpotAsset[i].locked;
                    iter->second.netAmountD = iter->second.totalAmountD;
                    iter->second.availableAmountD = vSpotAsset[i].free;
                    iter->second.underwayOrderValueD = GetUnderwayOrderValue(iter->second.asset, iter->second.frozenAmountD);
                }
            }
        }

        vector<binance::SpotOpenOrder>& vSpotOpenOrder = item->GetSpotOpenOrder();
        for (size_t i = 0; i < vSpotOpenOrder.size(); ++i) {
            string symbol = vSpotOpenOrder[i].symbol;
            string instKey = exchangeStr + "|" + symbol + "|SPOT";
            string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);

            double openQty = vSpotOpenOrder[i].origQty - vSpotOpenOrder[i].executedQty;
            auto iter = mSpotOpenOrder.find(key);
            if (iter != mSpotOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mSpotOpenOrder.insert(make_pair(key, openOrder));
            }
       }

        // ufuture
        vector<binance::UFutureAsset>& vUFutureAsset = item->GetUFutureAsset();
        for (size_t i = 0; i < vUFutureAsset.size(); ++i) {
            string ass = vUFutureAsset[i].assetType;
            double positionValue = item->GetUAssetPositionValue(ass);
            double floatAmount = item->GetUFloatAmount(ass);
            auto iter = mUFutureAsset.find(ass);
            if (iter == mUFutureAsset.end()) {
                igmonitor::Asset asset;
                asset.asset = vUFutureAsset[i].assetType;
                asset.positionValueD = positionValue;
                asset.totalAmountD = vUFutureAsset[i].walletBalance;
                //asset.floatAmountD = vUFutureAsset[i].unrealizedProfit;
                asset.floatAmountD = floatAmount;
                asset.marginAmountD = vUFutureAsset[i].initialMargin; // vUFutureAsset[i].initialMargin + vUFutureAsset[i].maintMargin;
                asset.frozenMarginAmountD = vUFutureAsset[i].openOrderInitialMargin;
                asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
                asset.availableAmountD = vUFutureAsset[i].availableBalance;
                //asset.underwayOrderValueD = GetUnderwayOrderValue(asset.asset, asset.frozenMarginAmountD);
                if (fabs(asset.netAmountD) > MINDOUBLE) {
                    asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << asset.positionValueD << " totalAmountD:" << asset.totalAmountD << " floatAmountD:" << asset.floatAmountD << " netAmountD:" << asset.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
                mUFutureAsset.insert(make_pair(asset.asset, asset));
            } else {
                iter->second.positionValueD = positionValue;
                iter->second.totalAmountD = vUFutureAsset[i].walletBalance;
                //iter->second.floatAmountD = vUFutureAsset[i].unrealizedProfit;
                iter->second.floatAmountD = floatAmount;
                iter->second.marginAmountD = vUFutureAsset[i].initialMargin;  // vUFutureAsset[i].initialMargin + vUFutureAsset[i].maintMargin;
                iter->second.frozenMarginAmountD = vUFutureAsset[i].openOrderInitialMargin;
                iter->second.netAmountD = iter->second.totalAmountD + iter->second.floatAmountD;
                iter->second.availableAmountD = vUFutureAsset[i].availableBalance;
                //iter->second.underwayOrderValueD = GetUnderwayOrderValue(iter->second.asset, iter->second.frozenMarginAmountD);
                if (fabs(iter->second.netAmountD) > MINDOUBLE) {
                    iter->second.realLeverageRatioD = fabs(iter->second.positionValueD / iter->second.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << iter->second.positionValueD << " totalAmountD:" << iter->second.totalAmountD << " floatAmountD:" << iter->second.floatAmountD << " netAmountD:" << iter->second.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
            }
        }

        vector<binance::UFuturePosition>& vUFuturePosition = item->GetUFuturePosition();
        for (size_t i = 0; i < vUFuturePosition.size(); ++i) {
            string symbol = vUFuturePosition[i].symbol;
            string positionSide = vUFuturePosition[i].positionSide;
            double liquidationPrice = item->GetUPositionLiquidationPrice(symbol, positionSide);
            double longFrozenPosition = 0.0;
            double shortFrozenPosition = 0.0;
            item->GetULongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
            auto iter = mUFuturePosition.find(symbol);
            if (iter == mUFuturePosition.end()) {
                igmonitor::Position position;
                position.symbol = vUFuturePosition[i].symbol;
                position.netPositionD = vUFuturePosition[i].positionAmt;
                position.netAvgPriceD = vUFuturePosition[i].entryPrice;
                position.floatAmountD = vUFuturePosition[i].unrealizedProfit;
                position.liquidationPrice = liquidationPrice;
                string instKey = exchangeStr + "|" + position.symbol + "|FUTURES";
                position.key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
                position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                position.underwayAbsPositioD = fabs(position.underwayNetPositionD);
                mUFuturePosition.insert(make_pair(position.symbol, position));
            } else {
                iter->second.netPositionD = vUFuturePosition[i].positionAmt;
                iter->second.netAvgPriceD = vUFuturePosition[i].entryPrice;
                iter->second.floatAmountD = vUFuturePosition[i].unrealizedProfit;
                iter->second.liquidationPrice = liquidationPrice;
                iter->second.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                iter->second.underwayAbsPositioD = fabs(iter->second.underwayNetPositionD);
            }
        }

        vector<binance::FutureOpenOrder>& vUFutureOpenOrder = item->GetUOpenOrder();
        for (size_t i = 0; i < vUFutureOpenOrder.size(); ++i) {
            string symbol = vUFutureOpenOrder[i].symbol;
            string instKey = exchangeStr + "|" + symbol + "|FUTURES";
            string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);

            double openQty = vUFutureOpenOrder[i].origQty - vUFutureOpenOrder[i].executedQty;
            auto iter = mUFutureOpenOrder.find(key);
            if (iter != mUFutureOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mUFutureOpenOrder.insert(make_pair(key, openOrder));
            }
        }


        // cfuture
        vector<binance::CFutureAsset>& vCFutureAsset = item->GetCFutureAsset();
        for (size_t i = 0; i < vCFutureAsset.size(); ++i) {
            string ass = vCFutureAsset[i].assetType;
            double positionValue = item->GetCAssetPositionValue(ass);
            double floatAmount = item->GetCFloatAmount(ass);
            auto iter = mCFutureAsset.find(ass);
            if (iter == mCFutureAsset.end()) {
                igmonitor::Asset asset;
                asset.asset = vCFutureAsset[i].assetType;
                asset.positionValueD = positionValue;
                asset.totalAmountD = vCFutureAsset[i].walletBalance;
                //asset.floatAmountD = vCFutureAsset[i].unrealizedProfit;
                asset.floatAmountD = floatAmount;
                asset.marginAmountD = vCFutureAsset[i].initialMargin;  // vCFutureAsset[i].initialMargin + vCFutureAsset[i].maintMargin;
                asset.frozenMarginAmountD = vCFutureAsset[i].openOrderInitialMargin;
                asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
                asset.availableAmountD = vCFutureAsset[i].availableBalance;
                //asset.underwayOrderValueD = GetUnderwayOrderValue(asset.asset, asset.frozenMarginAmountD);
                if (fabs(asset.netAmountD) > MINDOUBLE) {
                    asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << asset.positionValueD << " totalAmountD:" << asset.totalAmountD << " floatAmountD:" << asset.floatAmountD << " netAmountD:" << asset.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
                mCFutureAsset.insert(make_pair(asset.asset, asset));
            } else {
                iter->second.positionValueD = positionValue;
                iter->second.totalAmountD = vCFutureAsset[i].walletBalance;
                //iter->second.floatAmountD = vCFutureAsset[i].unrealizedProfit;
                iter->second.floatAmountD = floatAmount;
                iter->second.marginAmountD = vCFutureAsset[i].initialMargin;  // vCFutureAsset[i].initialMargin + vCFutureAsset[i].maintMargin;
                iter->second.frozenMarginAmountD = vCFutureAsset[i].openOrderInitialMargin;
                iter->second.netAmountD = iter->second.totalAmountD + iter->second.floatAmountD;
                iter->second.availableAmountD = vCFutureAsset[i].availableBalance;
                //iter->second.underwayOrderValueD = GetUnderwayOrderValue(iter->second.asset, iter->second.frozenMarginAmountD);
                if (fabs(iter->second.netAmountD) > MINDOUBLE) {
                    iter->second.realLeverageRatioD = fabs(iter->second.positionValueD / iter->second.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << iter->second.positionValueD << " totalAmountD:" << iter->second.totalAmountD << " floatAmountD:" << iter->second.floatAmountD << " netAmountD:" << iter->second.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
            }
        }

        vector<binance::CFuturePosition>& vCFuturePosition = item->GetCFuturePosition();
        for (size_t i = 0; i < vCFuturePosition.size(); ++i) {
            string symbol = vCFuturePosition[i].symbol;
            string positionSide = vCFuturePosition[i].positionSide;
            double liquidationPrice = item->GetCPositionLiquidationPrice(symbol, positionSide);
            double longFrozenPosition = 0.0;
            double shortFrozenPosition = 0.0;
            item->GetCLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
            auto iter = mCFuturePosition.find(symbol);
            if (iter == mCFuturePosition.end()) {
                igmonitor::Position position;
                position.symbol = vCFuturePosition[i].symbol;
                position.netPositionD = vCFuturePosition[i].positionAmt;
                position.netAvgPriceD = vCFuturePosition[i].entryPrice;
                position.floatAmountD = vCFuturePosition[i].unrealizedProfit;
                position.liquidationPrice = liquidationPrice;
                string instKey = exchangeStr + "|" + position.symbol + "|FUTURES";
                position.key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
                position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                position.underwayAbsPositioD = fabs(position.underwayNetPositionD);
                mCFuturePosition.insert(make_pair(position.symbol, position));
            } else {
                iter->second.netPositionD = vCFuturePosition[i].positionAmt;
                iter->second.netAvgPriceD = vCFuturePosition[i].entryPrice;
                iter->second.floatAmountD = vCFuturePosition[i].unrealizedProfit;
                iter->second.liquidationPrice = liquidationPrice;
                iter->second.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                iter->second.underwayAbsPositioD = fabs(iter->second.underwayNetPositionD);
            }
        }

        vector<binance::FutureOpenOrder>& vCFutureOpenOrder = item->GetCOpenOrder();
        for (size_t i = 0; i < vCFutureOpenOrder.size(); ++i) {
            string symbol = vCFutureOpenOrder[i].symbol;
            string instKey = exchangeStr + "|" + symbol + "|FUTURES";
            string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);

            double openQty = vCFutureOpenOrder[i].origQty - vCFutureOpenOrder[i].executedQty;
            auto iter = mCFutureOpenOrder.find(key);
            if (iter != mCFutureOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mCFutureOpenOrder.insert(make_pair(key, openOrder));
            }
        }

        // margin account
        vector<binance::MarginAsset>& vMarginAsset = item->GetMarginAsset();
        for (size_t i = 0; i < vMarginAsset.size(); ++i) {
            igmonitor::MarAsset marAsset;
            marAsset.asset = vMarginAsset[i].asset;
            marAsset.borrowed = vMarginAsset[i].borrowed;
            marAsset.free = vMarginAsset[i].free;
            marAsset.interest = vMarginAsset[i].interest;
            marAsset.locked = vMarginAsset[i].locked;
            marAsset.netAsset = vMarginAsset[i].netAsset;
            mMarAsset.insert(make_pair(marAsset.asset, marAsset));
        }
        
        binance::TotalMarginAsset& totalMarginAsset = item->GetTotalMarginAsset();
        totalMarAsset.marginLevel = totalMarginAsset.marginLevel;
        totalMarAsset.totalAssetOfBtc = totalMarginAsset.totalAssetOfBtc;
        totalMarAsset.totalLiabilityOfBtc = totalMarginAsset.totalLiabilityOfBtc;
        totalMarAsset.totalNetAssetOfBtc = totalMarginAsset.totalNetAssetOfBtc;


        // loan
        vector<binance::LoanBorrow>& vLoanBorrow = item->GetLoanBorrow();
        for (size_t i = 0; i < vLoanBorrow.size(); ++i) {
            igmonitor::LoBo lb;
            lb.loanCoin = vLoanBorrow[i].loanCoin;
            lb.loanAmount = vLoanBorrow[i].loanAmount;
            lb.collateralCoin = vLoanBorrow[i].collateralCoin;
            lb.collateralAmount = vLoanBorrow[i].collateralAmount;
            vLoBo.emplace_back(lb);
        }

        // unified
        if (unified == 1) {
            vector<binance::UnifyAsset>& vUnifyAsset = item->GetUnifyAsset();
            for (size_t i = 0; i < vUnifyAsset.size(); ++i) {
                string ass = vUnifyAsset[i].asset;
                double positionValue = item->GetUnifyPositionValue(ass);
                double floatAmount = item->GetUnifyFloatAmount(ass);
                igmonitor::Asset asset;
                asset.asset = vUnifyAsset[i].asset;
                asset.positionValueD = positionValue;
                asset.totalAmountD = vUnifyAsset[i].totalWalletBalance;
                asset.floatAmountD = floatAmount;
                asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
                if (fabs(asset.netAmountD) > MINDOUBLE) {
                    asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << asset.positionValueD << " totalAmountD:" << asset.totalAmountD << " floatAmountD:" << asset.floatAmountD << " netAmountD:" << asset.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
                mUFutureAsset[asset.asset] = asset;
            }

            vector<binance::UnifyPosition>& vUmUnifyPosition = item->GetUmUnifyPosition();
            for (size_t i = 0; i < vUmUnifyPosition.size(); ++i) {
                string symbol = vUmUnifyPosition[i].symbol;
                double longFrozenPosition = 0.0;
                double shortFrozenPosition = 0.0;
                item->GetUmUnifyLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
                igmonitor::Position position;
                position.symbol = vUmUnifyPosition[i].symbol;
                position.netPositionD = vUmUnifyPosition[i].positionAmt;
                position.netAvgPriceD = vUmUnifyPosition[i].entryPrice;
                position.floatAmountD = vUmUnifyPosition[i].unRealizedProfit;
                string instKey = exchangeStr + "|" + position.symbol + "|FUTURES";
                position.key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
                position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                position.underwayAbsPositioD = fabs(position.underwayNetPositionD);
                mUFuturePosition[position.symbol] = position;
            }

            vector<binance::UnifyPosition>& vCmUnifyPosition = item->GetCmUnifyPosition();
            for (size_t i = 0; i < vCmUnifyPosition.size(); ++i) {
                string symbol = vCmUnifyPosition[i].symbol;
                double longFrozenPosition = 0.0;
                double shortFrozenPosition = 0.0;
                item->GetCmUnifyLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);

                igmonitor::Position position;
                position.symbol = vCmUnifyPosition[i].symbol;
                position.netPositionD = vCmUnifyPosition[i].positionAmt;
                position.netAvgPriceD = vCmUnifyPosition[i].entryPrice;
                position.floatAmountD = vCmUnifyPosition[i].unRealizedProfit;
                string instKey = exchangeStr + "|" + position.symbol + "|FUTURES";
                position.key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
                position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                position.underwayAbsPositioD = fabs(position.underwayNetPositionD);
                mCFuturePosition[position.symbol] = position;
            }

            vector<binance::UnifyOpenOrder>& vUmUnifyOpenOrder = item->GetUmUnifyOpenOrder();
            for (size_t i = 0; i < vUmUnifyOpenOrder.size(); ++i) {
                string symbol = vUmUnifyOpenOrder[i].symbol;
                string instKey = exchangeStr + "|" + symbol + "|FUTURES";
                string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);

                double openQty = vUmUnifyOpenOrder[i].origQty - vUmUnifyOpenOrder[i].executedQty;
                auto iter = mUFutureOpenOrder.find(key);
                if (iter != mUFutureOpenOrder.end()) {
                    iter->second.openQty += fabs(openQty);
                } else {
                    igmonitor::OpenOrder openOrder;
                    openOrder.symbol = symbol;
                    openOrder.openQty = openQty;
                    mUFutureOpenOrder.insert(make_pair(key, openOrder));
                }
            }

            vector<binance::UnifyOpenOrder>& vCmUnifyOpenOrder = item->GetCmUnifyOpenOrder();
            for (size_t i = 0; i < vCmUnifyOpenOrder.size(); ++i) {
                string symbol = vCmUnifyOpenOrder[i].symbol;
                string instKey = exchangeStr + "|" + symbol + "|FUTURES";
                string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);

                double openQty = vCmUnifyOpenOrder[i].origQty - vCmUnifyOpenOrder[i].executedQty;
                auto iter = mCFutureOpenOrder.find(key);
                if (iter != mCFutureOpenOrder.end()) {
                    iter->second.openQty += fabs(openQty);
                } else {
                    igmonitor::OpenOrder openOrder;
                    openOrder.symbol = symbol;
                    openOrder.openQty = openQty;
                    mCFutureOpenOrder.insert(make_pair(key, openOrder));
                }
            }
        }
    }
}

void BinanceAccountItem::UpdateByCoinbaseAdapter() {
    CoinbaseAdapterItem* item = CoinbaseAdapterMgr::GetInstance().GetAdapterItem(customerId);
    if (item) {
        riskInfo.dUpdateTime = item->GetUpdateTime();
        adapterQuery = item->GetQueryStatus();
        adapterQueryErrMsg = item->GetQueryErrMsg();
        unified = item->isUnified();
    
        // spot
        vector<coinbase::SpotAsset>& vSpotAsset = item->GetSpotAsset();
        for (size_t i = 0; i < vSpotAsset.size(); ++i) {
            string ass = vSpotAsset[i].symbol;
            auto iter = mSpotAsset.find(ass);
            if (iter == mSpotAsset.end()) {
                igmonitor::Asset asset;
                asset.asset = vSpotAsset[i].symbol;
                asset.frozenAmountD = vSpotAsset[i].holds;
                asset.totalAmountD = vSpotAsset[i].amount;
                asset.netAmountD = asset.totalAmountD;
                asset.availableAmountD = vSpotAsset[i].amount - vSpotAsset[i].holds;
                asset.underwayOrderValueD = GetUnderwayOrderValue(asset.asset, asset.frozenAmountD);
                mSpotAsset.insert(make_pair(asset.asset, asset));
            } else {
                iter->second.frozenAmountD = vSpotAsset[i].holds;
                iter->second.totalAmountD = vSpotAsset[i].amount;
                iter->second.netAmountD = iter->second.totalAmountD;
                iter->second.availableAmountD = vSpotAsset[i].amount - vSpotAsset[i].holds;
                iter->second.underwayOrderValueD = GetUnderwayOrderValue(iter->second.asset, iter->second.frozenAmountD);
            }
        }
    }
}

void BinanceAccountItem::UpdateByGateioAdapter() {
    GateioAdapterItem* item = GateioAdapterMgr::GetInstance().GetAdapterItem(customerId);
    if (item) {
        riskInfo.dUpdateTime = item->GetUpdateTime();
        adapterQuery = item->GetQueryStatus();
        adapterQueryErrMsg = item->GetQueryErrMsg();
        unified = item->isUnified();

        gateio::CrossMarginAccountTotal& crossMarginAccountTotal = item->GetCrossMarginAccountTotal();
        totalMarginBalance = crossMarginAccountTotal.totalMarginBalance;
        initialMarginRate = crossMarginAccountTotal.totalInitialMarginRate;
    
        // spot
        vector<gateio::SpotAsset>& vSpotAsset = item->GetSpotAsset();
        for (size_t i = 0; i < vSpotAsset.size(); ++i) {
            string ass = vSpotAsset[i].currency;
            auto iter = mSpotAsset.find(ass);
            if (iter == mSpotAsset.end()) {
                igmonitor::Asset asset;
                asset.asset = vSpotAsset[i].currency;
                asset.frozenAmountD = vSpotAsset[i].locked;
                asset.totalAmountD = vSpotAsset[i].available + vSpotAsset[i].locked;
                asset.netAmountD = asset.totalAmountD;
                asset.availableAmountD = vSpotAsset[i].available;
                asset.underwayOrderValueD = GetUnderwayOrderValue(asset.asset, asset.frozenAmountD);
                mSpotAsset.insert(make_pair(asset.asset, asset));
            } else {
                iter->second.frozenAmountD = vSpotAsset[i].locked;
                iter->second.totalAmountD = vSpotAsset[i].available + vSpotAsset[i].locked;
                iter->second.netAmountD = iter->second.totalAmountD;
                iter->second.availableAmountD = vSpotAsset[i].available;
                iter->second.underwayOrderValueD = GetUnderwayOrderValue(iter->second.asset, iter->second.frozenAmountD);
            }
        }

        vector<gateio::FutureAsset>& vDeliveryAsset = item->GetDeliveryAsset();
        for (size_t i = 0; i < vDeliveryAsset.size(); ++i) {
            string ass = vDeliveryAsset[i].currency;
            double positionValue = item->GetDeliveryPositionValue(ass);
            double floatAmount = item->GetDeliveryFloatAmount(ass);
            auto iter = mDeliveryAsset.find(ass);
            if (iter == mDeliveryAsset.end()) {
                igmonitor::Asset asset;
                asset.asset = vDeliveryAsset[i].currency;
                asset.positionValueD = positionValue;
                asset.totalAmountD = vDeliveryAsset[i].total;
                //asset.floatAmountD = vDeliveryAsset[i].unrealisedPnl;
                asset.floatAmountD = floatAmount;
                asset.marginAmountD = vDeliveryAsset[i].positionInitialMargin;
                asset.frozenMarginAmountD = vDeliveryAsset[i].orderMargin;
                asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
                asset.availableAmountD = vDeliveryAsset[i].available;
                //asset.underwayOrderValueD = GetUnderwayOrderValue(asset.asset, asset.frozenMarginAmountD);
                if (fabs(asset.netAmountD) > MINDOUBLE) {
                    asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << asset.positionValueD << " totalAmountD:" << asset.totalAmountD << " floatAmountD:" << asset.floatAmountD << " netAmountD:" << asset.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
                mDeliveryAsset.insert(make_pair(asset.asset, asset));
            } else {
                iter->second.positionValueD = positionValue;
                iter->second.totalAmountD = vDeliveryAsset[i].total;
                //iter->second.floatAmountD = vDeliveryAsset[i].unrealisedPnl;
                iter->second.floatAmountD = floatAmount;
                iter->second.marginAmountD = vDeliveryAsset[i].positionInitialMargin;
                iter->second.frozenMarginAmountD = vDeliveryAsset[i].orderMargin;
                iter->second.netAmountD = iter->second.totalAmountD + iter->second.floatAmountD;                
                iter->second.availableAmountD = vDeliveryAsset[i].available;
                //iter->second.underwayOrderValueD = GetUnderwayOrderValue(iter->second.asset, iter->second.frozenMarginAmountD);
                if (fabs(iter->second.netAmountD) > MINDOUBLE) {
                    iter->second.realLeverageRatioD = fabs(iter->second.positionValueD / iter->second.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << iter->second.positionValueD << " totalAmountD:" << iter->second.totalAmountD << " floatAmountD:" << iter->second.floatAmountD << " netAmountD:" << iter->second.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
            }
        }

        vector<gateio::FuturePosition>& vDeliveryPosition = item->GetDeliveryPosition();
        for (size_t i = 0; i < vDeliveryPosition.size(); ++i) {
            string symbol = vDeliveryPosition[i].contract;
            double longFrozenPosition = 0.0;
            double shortFrozenPosition = 0.0;
            item->GetDeliveryLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
            auto iter = mDeliveryPosition.find(symbol);
            if (iter == mDeliveryPosition.end()) {
                igmonitor::Position position;
                position.symbol = vDeliveryPosition[i].contract;
                position.netPositionD = vDeliveryPosition[i].size;
                position.netAvgPriceD = vDeliveryPosition[i].entryPrice;
                position.floatAmountD = vDeliveryPosition[i].unrealisedPnl;
                position.liquidationPrice = vDeliveryPosition[i].liqPrice;
                string instKey = exchangeStr + "|" + position.symbol + "|FUTURES";
                position.key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
                position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                position.underwayAbsPositioD = fabs(position.underwayNetPositionD);
                mDeliveryPosition.insert(make_pair(position.symbol, position));
            } else {
                iter->second.netPositionD = vDeliveryPosition[i].size;
                iter->second.netAvgPriceD = vDeliveryPosition[i].entryPrice;
                iter->second.floatAmountD = vDeliveryPosition[i].unrealisedPnl;
                iter->second.liquidationPrice = vDeliveryPosition[i].liqPrice;
                iter->second.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                iter->second.underwayAbsPositioD = fabs(iter->second.underwayNetPositionD);
            }
        }

        vector<gateio::FutureOrder>& vDeliveryOpenOrder = item->GetDeliveryOpenOrder();
        for (size_t i = 0; i < vDeliveryOpenOrder.size(); ++i) {
            string symbol = vDeliveryOpenOrder[i].contract;
            string instKey = exchangeStr + "|" + symbol + "|FUTURES";
            string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
            double openQty = vDeliveryOpenOrder[i].left;
            auto iter = mDeliveryOpenOrder.find(key);
            if (iter != mDeliveryOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mDeliveryOpenOrder.insert(make_pair(key, openOrder));
            }
        }


        vector<gateio::FutureAsset>& vPerpetualAsset = item->GetPerpetualAsset();
        for (size_t i = 0; i < vPerpetualAsset.size(); ++i) {
            string ass = vPerpetualAsset[i].currency;
            double positionValue = item->GetPerpetualPositionValue(ass);
            double floatAmount = item->GetPerpetualFloatAmount(ass);
            auto iter = mPerpetualAsset.find(ass);
            if (iter == mPerpetualAsset.end()) {
                igmonitor::Asset asset;
                asset.asset = vPerpetualAsset[i].currency;
                asset.positionValueD = positionValue;
                asset.totalAmountD = vPerpetualAsset[i].total;
                //asset.floatAmountD = vPerpetualAsset[i].unrealisedPnl;
                asset.floatAmountD = floatAmount;
                asset.marginAmountD = vPerpetualAsset[i].positionInitialMargin;
                asset.frozenMarginAmountD = vPerpetualAsset[i].orderMargin;
                asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
                asset.availableAmountD = vPerpetualAsset[i].available;
                //asset.underwayOrderValueD = GetUnderwayOrderValue(asset.asset, asset.frozenMarginAmountD);
                if (fabs(asset.netAmountD) > MINDOUBLE) {
                    asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << asset.positionValueD << " totalAmountD:" << asset.totalAmountD << " floatAmountD:" << asset.floatAmountD << " netAmountD:" << asset.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
                mPerpetualAsset.insert(make_pair(asset.asset, asset));
            } else {
                iter->second.positionValueD = positionValue;
                iter->second.totalAmountD = vPerpetualAsset[i].total;
                //iter->second.floatAmountD = vPerpetualAsset[i].unrealisedPnl;
                iter->second.floatAmountD = floatAmount;
                iter->second.marginAmountD = vPerpetualAsset[i].positionInitialMargin;
                iter->second.frozenMarginAmountD = vPerpetualAsset[i].orderMargin;
                iter->second.netAmountD = iter->second.totalAmountD + iter->second.floatAmountD;                
                iter->second.availableAmountD = vPerpetualAsset[i].available;
                //iter->second.underwayOrderValueD = GetUnderwayOrderValue(iter->second.asset, iter->second.frozenMarginAmountD);
                if (fabs(iter->second.netAmountD) > MINDOUBLE) {
                    iter->second.realLeverageRatioD = fabs(iter->second.positionValueD / iter->second.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << iter->second.positionValueD << " totalAmountD:" << iter->second.totalAmountD << " floatAmountD:" << iter->second.floatAmountD << " netAmountD:" << iter->second.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
            }
        }

        vector<gateio::FuturePosition>& vPerpetualPosition = item->GetPerpetualPosition();
        for (size_t i = 0; i < vPerpetualPosition.size(); ++i) {
            string symbol = vPerpetualPosition[i].contract;
            double longFrozenPosition = 0.0;
            double shortFrozenPosition = 0.0;
            item->GetPerpetualLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
            auto iter = mPerpetualPosition.find(symbol);
            if (iter == mPerpetualPosition.end()) {
                igmonitor::Position position;
                position.symbol = vPerpetualPosition[i].contract;
                position.netPositionD = vPerpetualPosition[i].size;
                position.netAvgPriceD = vPerpetualPosition[i].entryPrice;
                position.floatAmountD = vPerpetualPosition[i].unrealisedPnl;
                position.liquidationPrice = vPerpetualPosition[i].liqPrice;
                string instKey = exchangeStr + "|" + position.symbol + "|FUTURES";
                position.key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
                position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                position.underwayAbsPositioD = fabs(position.underwayNetPositionD);
                mPerpetualPosition.insert(make_pair(position.symbol, position));
            } else {
                iter->second.netPositionD = vPerpetualPosition[i].size;
                iter->second.netAvgPriceD = vPerpetualPosition[i].entryPrice;
                iter->second.floatAmountD = vPerpetualPosition[i].unrealisedPnl;
                iter->second.liquidationPrice = vPerpetualPosition[i].liqPrice;
                iter->second.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                iter->second.underwayAbsPositioD = fabs(iter->second.underwayNetPositionD);
            }
        }

        vector<gateio::FutureOrder>& vPerpetualOpenOrder = item->GetPerpetualOpenOrder();
        for (size_t i = 0; i < vPerpetualOpenOrder.size(); ++i) {
            string symbol = vPerpetualOpenOrder[i].contract;
            string instKey = exchangeStr + "|" + symbol + "|FUTURES";
            string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
            double openQty = vPerpetualOpenOrder[i].left;
            auto iter = mPerpetualOpenOrder.find(key);
            if (iter != mPerpetualOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mPerpetualOpenOrder.insert(make_pair(key, openOrder));
            }
        }


        vector<gateio::CrossMarginAsset>& vCrossMarginAsset = item->GetCrossMarginAsset();
        for (size_t i = 0; i < vCrossMarginAsset.size(); ++i) {
            string ass = vCrossMarginAsset[i].currency;
            double positionValue = item->GetPerpetualPositionValue(ass);
            double floatAmount = item->GetPerpetualFloatAmount(ass);
            double total = item->GetPerpetualAssetTotal(ass);
            igmonitor::Asset asset;
            asset.asset = vCrossMarginAsset[i].currency;
            asset.positionValueD = positionValue;
            asset.totalAmountD = vCrossMarginAsset[i].available + vCrossMarginAsset[i].freeze + total;
            asset.floatAmountD = floatAmount;
            // asset.marginAmountD = vPerpetualAsset[i].positionInitialMargin;
            // asset.frozenMarginAmountD = vPerpetualAsset[i].orderMargin;
            asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
            asset.availableAmountD = vCrossMarginAsset[i].available;
            if (fabs(asset.netAmountD) > MINDOUBLE) {
                asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
                stringstream ss;
                ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << asset.positionValueD << " totalAmountD:" << asset.totalAmountD << " floatAmountD:" << asset.floatAmountD << " netAmountD:" << asset.netAmountD;
                LOG_INFO("realLeverageRatioD: %s", ss.str().c_str());
            }
            // mCrossMarAsset.insert(make_pair(asset.asset, asset));
            if (unified == 1) {
                mPerpetualAsset[asset.asset] = asset; // 先使用统一账户的资产信息当作永续合约的资产信息（这样改动最小）
            } 
        }
    }
}

void BinanceAccountItem::UpdateByBybitAdapter() {
    BybitAdapterItem* item = BybitAdapterMgr::GetInstance().GetAdapterItem(customerId);
    if (item) {
        riskInfo.dUpdateTime = item->GetUpdateTime();
        adapterQuery = item->GetQueryStatus();
        adapterQueryErrMsg = item->GetQueryErrMsg();
        unified = item->isUnified();

        bybit::TotalAccountInfo& totalAccountInfo = item->GetTotalAccountInfo();
        totalMarginBalance = totalAccountInfo.totalMarginBalance;
   
        // asset
        vector<bybit::Asset>& vAsset = item->GetAsset();
        for (size_t i = 0; i < vAsset.size(); ++i) {
            string ass = vAsset[i].coin;
            double positionValue = item->GetPositionValue(ass);
            double floatAmount = item->GetFloatAmount(ass);
            auto iter = mUFutureAsset.find(ass);
            if (iter == mUFutureAsset.end()) {
                igmonitor::Asset asset;
                asset.asset = vAsset[i].coin;
                asset.positionValueD = positionValue;
                asset.totalAmountD = vAsset[i].walletBalance;
                //asset.floatAmountD = vAsset[i].unrealisedPnl;
                asset.floatAmountD = floatAmount;
                asset.marginAmountD = vAsset[i].totalPositionIM;
                asset.frozenMarginAmountD = vAsset[i].totalOrderIM;
                asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
                // asset.availableAmountD = vAsset[i].available;
                //asset.underwayOrderValueD = GetUnderwayOrderValue(asset.asset, asset.frozenMarginAmountD);
                if (fabs(asset.netAmountD) > MINDOUBLE) {
                    asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << asset.positionValueD << " totalAmountD:" << asset.totalAmountD << " floatAmountD:" << asset.floatAmountD << " netAmountD:" << asset.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
                mUFutureAsset.insert(make_pair(asset.asset, asset));
            } else {
                iter->second.positionValueD = positionValue;
                iter->second.totalAmountD = vAsset[i].walletBalance;
                //iter->second.floatAmountD = vAsset[i].unrealisedPnl;
                iter->second.floatAmountD = floatAmount;
                iter->second.marginAmountD = vAsset[i].totalPositionIM;
                iter->second.frozenMarginAmountD = vAsset[i].totalOrderIM;
                iter->second.netAmountD = iter->second.totalAmountD + iter->second.floatAmountD;              
                // iter->second.availableAmountD = vAsset[i].available;
                //iter->second.underwayOrderValueD = GetUnderwayOrderValue(iter->second.asset, iter->second.frozenMarginAmountD);
                if (fabs(iter->second.netAmountD) > MINDOUBLE) {
                    iter->second.realLeverageRatioD = fabs(iter->second.positionValueD / iter->second.netAmountD);
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << ass << " positionValueD:" << iter->second.positionValueD << " totalAmountD:" << iter->second.totalAmountD << " floatAmountD:" << iter->second.floatAmountD << " netAmountD:" << iter->second.netAmountD;
                    LOG_INFO("realLeverageRatioD: %s", ss.str().c_str()); 
                }
            }
        }

        vector<bybit::Position>& vPosition = item->GetPosition();
        for (size_t i = 0; i < vPosition.size(); ++i) {
            string symbol = vPosition[i].symbol;
            double longFrozenPosition = 0.0;
            double shortFrozenPosition = 0.0;
            item->GetLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
            auto iter = mUFuturePosition.find(symbol);
            if (iter == mUFuturePosition.end()) {
                igmonitor::Position position;
                position.symbol = vPosition[i].symbol;
                position.netPositionD = vPosition[i].size;
                position.netAvgPriceD = vPosition[i].avgPrice;
                position.floatAmountD = vPosition[i].unrealisedPnl;
                position.liquidationPrice = vPosition[i].liqPrice;
                string instKey = exchangeStr + "|" + position.symbol + "|FUTURES";
                position.key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
                position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                position.underwayAbsPositioD = fabs(position.underwayNetPositionD);
                mUFuturePosition.insert(make_pair(position.symbol, position));
            } else {
                iter->second.netPositionD = vPosition[i].size;
                iter->second.netAvgPriceD = vPosition[i].avgPrice;
                iter->second.floatAmountD = vPosition[i].unrealisedPnl;
                iter->second.liquidationPrice = vPosition[i].liqPrice;
                iter->second.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                iter->second.underwayAbsPositioD = fabs(iter->second.underwayNetPositionD);
            }
        }

        vector<bybit::Order>& vOpenOrder = item->GetOpenOrder();
        for (size_t i = 0; i < vOpenOrder.size(); ++i) {
            string symbol = vOpenOrder[i].symbol;
            string instKey = exchangeStr + "|" + symbol + "|FUTURES";
            string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
            double openQty = vOpenOrder[i].qty - vOpenOrder[i].cumExecQty;
            auto iter = mUFutureOpenOrder.find(key);
            if (iter != mUFutureOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mUFutureOpenOrder.insert(make_pair(key, openOrder));
            }
        }

    }
}

void BinanceAccountItem::ClearZero() {
    // clear asset
    for (auto iter = mSpotAsset.begin(); iter != mSpotAsset.end();) {
        if (fabs(iter->second.netAmountS) <= MINDOUBLE && fabs(iter->second.initialAmount) <= MINDOUBLE) {
            mSpotAsset.erase(iter++);
        } else {
            iter++;
        }
    }

    for (auto iter = mUFutureAsset.begin(); iter != mUFutureAsset.end();) {
        if (fabs(iter->second.netAmountS) <= MINDOUBLE && fabs(iter->second.initialAmount) <= MINDOUBLE) {
            mUFutureAsset.erase(iter++);
        } else {
            iter++;
        }
    }

    for (auto iter = mCFutureAsset.begin(); iter != mCFutureAsset.end();) {
        if (fabs(iter->second.netAmountS) <= MINDOUBLE && fabs(iter->second.initialAmount) <= MINDOUBLE) {
            mCFutureAsset.erase(iter++);
        } else {
            iter++;
        }
    }

    // clear position
    for (auto iter = mUFuturePosition.begin(); iter != mUFuturePosition.end();) {
        if (fabs(iter->second.netPositionS) <= MINDOUBLE && fabs(iter->second.netPositionD) <= MINDOUBLE) {
            mUFuturePosition.erase(iter++);
        } else {
            iter++;
        }
    }

    for (auto iter = mCFuturePosition.begin(); iter != mCFuturePosition.end();) {
        if (fabs(iter->second.netPositionS) <= MINDOUBLE && fabs(iter->second.netPositionD) <= MINDOUBLE) {
            mCFuturePosition.erase(iter++);
        } else {
            iter++;
        }
    }
}

void BinanceAccountItem::Clear() {
    mSavAsset.clear();
    mSpotAsset.clear();
    mUFutureAsset.clear();
    mCFutureAsset.clear();
    mDeliveryAsset.clear();
    mPerpetualAsset.clear();
    mUFuturePosition.clear();
    mCFuturePosition.clear();
    mDeliveryPosition.clear();
    mPerpetualPosition.clear();
    mSpotOpenOrder.clear();
    mUFutureOpenOrder.clear();
    mCFutureOpenOrder.clear();
    mDeliveryOpenOrder.clear();
    mPerpetualOpenOrder.clear();
    mMarAsset.clear();
    mDeliveryAsset.clear();
    mPerpetualAsset.clear();
    mCrossMarAsset.clear();
    mTotalAsset.clear();
    mExposure.clear();
    mPositionFundingRate.clear();
    vLoBo.clear();
}

void BinanceAccountItem::CalculateTotalAsset() {
    for (auto iter = mSpotAsset.begin(); iter != mSpotAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mUFutureAsset.begin(); iter != mUFutureAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mCFutureAsset.begin(); iter != mCFutureAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mMarAsset.begin(); iter != mMarAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mSavAsset.begin(); iter != mSavAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mDeliveryAsset.begin(); iter != mDeliveryAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mPerpetualAsset.begin(); iter != mPerpetualAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mCrossMarAsset.begin(); iter != mCrossMarAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (size_t i = 0; i < vLoBo.size(); ++i) {
        UpdateTotalAsset(vLoBo[i]);
    }

    for (auto iter = mTotalAsset.begin(); iter != mTotalAsset.end(); ++iter) {
        if (fabs(iter->second.netAmountS) > MINDOUBLE) {
            iter->second.realLeverageRatioS = fabs(iter->second.positionValueS / iter->second.netAmountS);
        }
        if (fabs(iter->second.netAmountD) > MINDOUBLE) {
            iter->second.realLeverageRatioD = fabs(iter->second.positionValueD / iter->second.netAmountD);
        }
    }
}

void BinanceAccountItem::UpdateTotalAsset(igmonitor::Asset& asset) {
    auto iter = mTotalAsset.find(asset.asset);
    if (iter != mTotalAsset.end()) {
        iter->second.positionValueS += asset.positionValueS;
        iter->second.positionValueD += asset.positionValueD;
        iter->second.transferAmount += asset.transferAmount;
        iter->second.netAmountS += asset.netAmountS;
        iter->second.netAmountD += asset.netAmountD;
        iter->second.availableAmountS += asset.availableAmountS;
        iter->second.availableAmountD += asset.availableAmountD;
        iter->second.totalAmountS += asset.totalAmountS;
        iter->second.totalAmountD += asset.totalAmountD;
        iter->second.frozenAmountS += asset.frozenAmountS;
        iter->second.frozenAmountD += asset.frozenAmountD;
        iter->second.floatAmountS += asset.floatAmountS;
        iter->second.floatAmountD += asset.floatAmountD;
        iter->second.marginAmountS += asset.marginAmountS;
        iter->second.marginAmountD += asset.marginAmountD;
        iter->second.frozenMarginAmountS += asset.frozenMarginAmountS;
        iter->second.frozenMarginAmountD += asset.frozenMarginAmountD;
        iter->second.initialAmount += asset.initialAmount;
    } else {
        mTotalAsset.insert(make_pair(asset.asset, asset));
    }
}

void BinanceAccountItem::UpdateTotalAsset(igmonitor::MarAsset& asset) {
    auto iter = mTotalAsset.find(asset.asset);
    if (iter != mTotalAsset.end()) {
        iter->second.totalAmountD += asset.free;
        iter->second.netAmountD += asset.netAsset;
    } else {
        igmonitor::Asset ass;
        ass.asset = asset.asset;
        ass.totalAmountD = asset.free;
        ass.netAmountD = asset.netAsset;
        mTotalAsset.insert(make_pair(ass.asset, ass));
    }
}

void BinanceAccountItem::UpdateTotalAsset(igmonitor::SavAsset& asset) {
    auto iter = mTotalAsset.find(asset.asset);
    if (iter != mTotalAsset.end()) {
        iter->second.totalAmountD += asset.amountD;
        iter->second.netAmountD += asset.amountD;
    } else {
        igmonitor::Asset ass;
        ass.asset = asset.asset;
        ass.totalAmountD = asset.amountD;
        ass.netAmountD = asset.amountD;
        mTotalAsset.insert(make_pair(ass.asset, ass));
    }
}

void BinanceAccountItem::UpdateTotalAsset(igmonitor::LoBo& lb) {
    auto iter = mTotalAsset.find(lb.loanCoin);
    if (iter != mTotalAsset.end()) {
        iter->second.totalAmountD -= lb.loanAmount;
        iter->second.netAmountD -= lb.loanAmount;
    } else {
        igmonitor::Asset ass;
        ass.asset = lb.loanCoin;
        ass.totalAmountD = -lb.loanAmount;
        ass.netAmountD = -lb.loanAmount;
        mTotalAsset.insert(make_pair(ass.asset, ass));
    }

    auto it = mTotalAsset.find(lb.collateralCoin);
    if (it != mTotalAsset.end()) {
        it->second.totalAmountD += lb.collateralAmount;
        it->second.netAmountD += lb.collateralAmount;
    } else {
        igmonitor::Asset ass;
        ass.asset = lb.collateralCoin;
        ass.totalAmountD = lb.collateralAmount;
        ass.netAmountD = lb.collateralAmount;
        mTotalAsset.insert(make_pair(ass.asset, ass));
    }
}

void BinanceAccountItem::CalculateExposure() {
    for (auto iter = mUFuturePosition.begin(); iter != mUFuturePosition.end(); ++iter) {
        UpdateExposure("UFuture", iter->second);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(iter->second.key);
        if (info.instrumentType == "SWAP" || info.instrumentType == "InstType_USDT_SWAP") {
            UpdatePositionFundingRate(iter->second);
        }
    }
   
    for (auto iter = mCFuturePosition.begin(); iter != mCFuturePosition.end(); ++iter) {
        UpdateExposure("CFuture", iter->second);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(iter->second.key);
        if (info.instrumentType == "SWAP" ||info.instrumentType == "InstType_BTC_SWAP") {
            UpdatePositionFundingRate(iter->second);
        }
    }

    for (auto iter = mDeliveryPosition.begin(); iter != mDeliveryPosition.end(); ++iter) {
        string key = iter->second.key;
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (info.instrumentType == "InstType_USDT_FUTURES" || info.calculateType == 0) {
            UpdateExposure("UFuture", iter->second);
        } else if (info.instrumentType == "InstType_BTC_FUTURES" || info.calculateType == 1) {
            UpdateExposure("CFuture", iter->second);
        }
    }

    for (auto iter = mPerpetualPosition.begin(); iter != mPerpetualPosition.end(); ++iter) {
        string key = iter->second.key;
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (info.instrumentType == "InstType_USDT_SWAP" || info.calculateType == 0) {
            UpdateExposure("UFuture", iter->second);
        } else if (info.instrumentType == "InstType_BTC_SWAP" || info.calculateType == 1) {
            UpdateExposure("CFuture", iter->second);
        }

        UpdatePositionFundingRate(iter->second);
    }
 
    for (auto iter = mTotalAsset.begin(); iter != mTotalAsset.end(); ++iter) {
        UpdateExposure(iter->second);
    }
  
    for (auto iter = mExposure.begin(); iter != mExposure.end(); ++iter) {
        string asset = iter->first;
        if (asset == baseAsset) {
            iter->second.exposureValueS = iter->second.exposureAmountS;
            iter->second.exposureValueD = iter->second.exposureAmountD;
            iter->second.deltaAmountS = iter->second.exposureValueS - iter->second.initialAmount;
            iter->second.deltaAmountD = iter->second.exposureValueD - iter->second.initialAmount;
            iter->second.deltaValueS = iter->second.deltaAmountS;
            iter->second.deltaValueD = iter->second.deltaAmountD;
            iter->second.initialValue = iter->second.initialAmount;
        } else {
            if (asset == "USDT" || asset == "USD") {
                double price = 0.0;
                if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                    price = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                } else if (exchangeStr == "COINBASE") {
                    price = CoinbaseMdMgr::GetInstance().GetAssetPrice(baseAsset);
                }
                
                if (price > MINDOUBLE) {
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << asset << " price:" << price << " exposureAmountD:" << iter->second.exposureAmountD;
                    LOG_INFO("CalculateExposure: %s", ss.str().c_str());  
                    iter->second.exposureValueS = iter->second.exposureAmountS / price;
                    iter->second.exposureValueD = iter->second.exposureAmountD / price;
                    iter->second.deltaAmountS = iter->second.exposureAmountS - iter->second.initialAmount;
                    iter->second.deltaAmountD = iter->second.exposureAmountD - iter->second.initialAmount;
                    iter->second.deltaValueS = iter->second.deltaAmountS / price;
                    iter->second.deltaValueD = iter->second.deltaAmountD / price;
                    iter->second.initialValue = iter->second.initialAmount / price;
                }
            } else {
                string key = exchangeStr + "|" + asset + "-" + baseAsset + "|SPOT";
                double price = 0.0;
                if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                    price = BinanceMdMgr::GetInstance().GetMidPrice(key);  // 首先获取币对现货价格
                } else if (exchangeStr == "COINBASE") {
                    price = CoinbaseMdMgr::GetInstance().GetMidPrice(key);  // 首先获取币对现货价格
                }
                
                if (price > MINDOUBLE) {
                    stringstream ss;
                    ss << "accountId:" << customerId << " asset:" << asset << " price:" << price << " exposureAmountD:" << iter->second.exposureAmountD;
                    LOG_INFO("CalculateExposure: %s", ss.str().c_str());  
                    iter->second.exposureValueS = iter->second.exposureAmountS * price;
                    iter->second.exposureValueD = iter->second.exposureAmountD * price;
                    iter->second.deltaAmountS = iter->second.exposureAmountS - iter->second.initialAmount;
                    iter->second.deltaAmountD = iter->second.exposureAmountD - iter->second.initialAmount;
                    iter->second.deltaValueS = iter->second.deltaAmountS * price;
                    iter->second.deltaValueD = iter->second.deltaAmountD * price;
                    iter->second.initialValue = iter->second.initialAmount * price;
                } else { // 获取对应usdt的价格，再换算成币
                    if (baseAsset == "USDT" || baseAsset == "USD") {
                        double price = 0.0;
                        if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                            price = BinanceMdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                        } else if (exchangeStr == "COINBASE") {
                            price = CoinbaseMdMgr::GetInstance().GetAssetPrice(asset);
                        }

                        if (price > MINDOUBLE) {
                            stringstream ss;
                            ss << "accountId:" << customerId << " asset:" << asset << " price:" << price << " exposureAmountD:" << iter->second.exposureAmountD;
                            LOG_INFO("CalculateExposure: %s", ss.str().c_str());  
                            iter->second.exposureValueS = iter->second.exposureAmountS * price;
                            iter->second.exposureValueD = iter->second.exposureAmountD * price;
                            iter->second.deltaAmountS = iter->second.exposureAmountS - iter->second.initialAmount;
                            iter->second.deltaAmountD = iter->second.exposureAmountD - iter->second.initialAmount;
                            iter->second.deltaValueS = iter->second.deltaAmountS * price;
                            iter->second.deltaValueD = iter->second.deltaAmountD * price;
                            iter->second.initialValue = iter->second.initialAmount * price;
                        }
                    } else {
                        double priceU = 0.0;
                        double priceBaseAsset = 0.0;

                        if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                            priceU = BinanceMdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                            priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                        } else if (exchangeStr == "COINBASE") {
                            priceU = CoinbaseMdMgr::GetInstance().GetAssetPrice(asset);
                            priceBaseAsset = CoinbaseMdMgr::GetInstance().GetAssetPrice(baseAsset);
                        }

                        if (priceBaseAsset > MINDOUBLE) {
                            stringstream ss;
                            ss << "accountId:" << customerId << " asset:" << asset << " priceU:" << priceU << " baseAsset:" << baseAsset << " priceBaseAsset:" << priceBaseAsset << " exposureAmountD:" << iter->second.exposureAmountD;
                            LOG_INFO("CalculateExposure: %s", ss.str().c_str());  
                            iter->second.exposureValueS = iter->second.exposureAmountS * priceU / priceBaseAsset;
                            iter->second.exposureValueD = iter->second.exposureAmountD * priceU / priceBaseAsset;
                            iter->second.deltaAmountS = iter->second.exposureAmountS - iter->second.initialAmount;
                            iter->second.deltaAmountD = iter->second.exposureAmountD - iter->second.initialAmount;
                            iter->second.deltaValueS = iter->second.deltaAmountS * priceU / priceBaseAsset;
                            iter->second.deltaValueD = iter->second.deltaAmountD * priceU / priceBaseAsset;
                            iter->second.initialValue = iter->second.initialAmount * priceU / priceBaseAsset;
                        }
                    }
                }
            }
        }
    }
}

void BinanceAccountItem::UpdateExposure(string positionType, igmonitor::Position& position) {
    string key = position.key;
    double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
    InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
    string base = info.instLeft;
    string quote = info.instRight;

    stringstream ss;
    ss << "accountId:" << customerId << " key:" << key << " price:" << price << " netPositionD:" << position.netPositionD;
    LOG_INFO("UpdateExposure: %s", ss.str().c_str());  

    auto i = mExposure.find(base);
    if (i != mExposure.end()) {
        if (positionType == "UFuture") {
            i->second.exposureAmountS += position.netPositionS * info.multiple;
            i->second.exposureAmountD += position.netPositionD * info.multiple;
        } else if (positionType == "CFuture") {
            if (price > MINDOUBLE) {
                if (exchangeStr == "GATEIO") {
                    i->second.exposureAmountS += position.netPositionS * info.multipleVolume / price;
                    i->second.exposureAmountD += position.netPositionD * info.multipleVolume / price;
                } else {
                    i->second.exposureAmountS += position.netPositionS * info.multiple / price;
                    i->second.exposureAmountD += position.netPositionD * info.multiple / price;
                }
            }
        }
    } else {
        igmonitor::Exposure baseExposure;
        baseExposure.asset = base;
        if (positionType == "UFuture") {
            baseExposure.exposureAmountS = position.netPositionS * info.multiple ;
            baseExposure.exposureAmountD = position.netPositionD * info.multiple ;
        } else if (positionType == "CFuture") {
            if (price > MINDOUBLE) {
                if (exchangeStr == "GATEIO") {
                    baseExposure.exposureAmountS = position.netPositionS * info.multipleVolume / price;
                    baseExposure.exposureAmountD = position.netPositionD * info.multipleVolume / price;
                } else {
                    baseExposure.exposureAmountS = position.netPositionS * info.multiple / price;
                    baseExposure.exposureAmountD = position.netPositionD * info.multiple / price;
                }
            }
        }
        mExposure.insert(make_pair(base, baseExposure));
    }

    auto j = mExposure.find(quote);
    if (j != mExposure.end()) {
        if (positionType == "UFuture") {
            j->second.exposureAmountS -= position.netPositionS * price * info.multiple ;
            j->second.exposureAmountD -= position.netPositionD * price * info.multiple ;
        } else if (positionType == "CFuture") {
            if (exchangeStr == "GATEIO") {
                j->second.exposureAmountS -= position.netPositionS * info.multipleVolume;
                j->second.exposureAmountD -= position.netPositionD * info.multipleVolume;
            } else {
                j->second.exposureAmountS -= position.netPositionS * info.multiple;
                j->second.exposureAmountD -= position.netPositionD * info.multiple;
            }
        }
    } else {
        igmonitor::Exposure quoteExposure;
        quoteExposure.asset = quote;
        if (positionType == "UFuture") {
            quoteExposure.exposureAmountS = -position.netPositionS * price * info.multiple ;
            quoteExposure.exposureAmountD = -position.netPositionD * price * info.multiple ;
        } else if (positionType == "CFuture") {
            if (exchangeStr == "GATEIO") {
                quoteExposure.exposureAmountS = -position.netPositionS * info.multipleVolume;
                quoteExposure.exposureAmountD = -position.netPositionD * info.multipleVolume;
            } else {
                quoteExposure.exposureAmountS = -position.netPositionS * info.multiple;
                quoteExposure.exposureAmountD = -position.netPositionD * info.multiple;
            }
        }      
        mExposure.insert(make_pair(quote, quoteExposure));
    }
}

void BinanceAccountItem::UpdateExposure(igmonitor::Asset& asset) {
    stringstream ss;
    ss << "accountId:" << customerId << " asset:" << asset.asset << " netAmountD:" << asset.netAmountD;
    LOG_INFO("UpdateExposure: %s", ss.str().c_str());  
    auto iter = mExposure.find(asset.asset);
    if (iter != mExposure.end()) {
        iter->second.exposureAmountS += asset.netAmountS;
        iter->second.exposureAmountD += asset.netAmountD;
        iter->second.initialAmount = asset.initialAmount;
    } else {
        igmonitor::Exposure exposure;
        exposure.asset = asset.asset;
        exposure.exposureAmountS = asset.netAmountS;
        exposure.exposureAmountD = asset.netAmountD;
        exposure.initialAmount = asset.initialAmount;
        mExposure.insert(make_pair(asset.asset, exposure));
    }
}

void BinanceAccountItem::UpdatePositionFundingRate(igmonitor::Position& position) {
    if (fabs(position.netPositionD) < MINDOUBLE) {
        return;
    }
    
    igmonitor::PositionFundingRate pfr;
    pfr.symbol = position.symbol;
    pfr.netPosition = position.netPositionD;
    pfr.flag = position.netPositionD > 0 ? true : false;
    pfr.value = BinanceMdMgr::GetInstance().GetFundingRate(position.key);
    LOG_INFO("UpdatePositionFundingRate:  accountId: %d  key:%s  flag:%d  value:%f", customerId, position.key.c_str(), pfr.flag, pfr.value);
    mPositionFundingRate[position.key] = pfr;
}

double BinanceAccountItem::GetUnderwayOrderValue(string asset, double frozenAmount) {
    double underwayOrderValue = 0.0;
    if (asset == baseAsset) {
        underwayOrderValue = frozenAmount;
    } else {
        if (asset == "USDT" || asset == "USD") {
            double priceBaseAsset = 0.0;
            if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
            } else if (exchangeStr == "COINBASE") {
                priceBaseAsset = CoinbaseMdMgr::GetInstance().GetAssetPrice(baseAsset);
            }
            
            if (priceBaseAsset > MINDOUBLE) {
                underwayOrderValue = frozenAmount / priceBaseAsset;
            }
        } else {
            string key = exchangeStr + "|" + asset + "-" + baseAsset + "|SPOT";
            double price = 0.0;
            if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                price = BinanceMdMgr::GetInstance().GetMidPrice(key);
            } else if (exchangeStr == "COINBASE") {
                price = CoinbaseMdMgr::GetInstance().GetMidPrice(key);
            }
            
            if (price > 0) {
                underwayOrderValue = frozenAmount * price;
            } else {
                if (baseAsset == "USDT" || baseAsset == "USD") {
                    double price = 0.0;
                    if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                        price = BinanceMdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                    } else if (exchangeStr == "COINBASE") {
                        price = CoinbaseMdMgr::GetInstance().GetAssetPrice(asset);
                    }
                    
                    underwayOrderValue = frozenAmount * price;
                } else {
                    double priceU = 0.0;
                    double priceBaseAsset = 0.0;
                    if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                        priceU = BinanceMdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                        priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                    } else if (exchangeStr == "COINBASE") {
                        priceU = CoinbaseMdMgr::GetInstance().GetAssetPrice(asset);
                        priceBaseAsset = CoinbaseMdMgr::GetInstance().GetAssetPrice(baseAsset);
                    }

                    if (priceBaseAsset > MINDOUBLE) {
                        underwayOrderValue = frozenAmount * priceU / priceBaseAsset;
                    }
                }
            }
        }
    }
    return underwayOrderValue;
}

void BinanceAccountItem::CalculateRiskInfo() {
    riskInfo.tsLocal = GetCurrentTimeUs();
    totalExposure = 0.0;

    double netValueS = 0.0;
    double netValueD = 0.0;
    double yNetValue = 0.0;
    double totalAvailMargin = 0.0;
    double totalPositionValue = 0.0;
    for (auto iter = mTotalAsset.begin(); iter != mTotalAsset.end(); ++iter) {
        string asset = iter->first;
        double assetRate = MonitorConfig::GetInstance().GetAssetRate(exchangeStr, asset);
        if (asset == baseAsset) {
            yNetValue += iter->second.initialAmount;
            netValueS += iter->second.netAmountS;
            netValueD += iter->second.netAmountD;
            totalAvailMargin += iter->second.netAmountD;
            totalPositionValue += iter->second.positionValueD;
        } else {
            if (asset == "USDT" || asset == "USD") {
                double price = 0.0;
                if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                    price = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                } else if (exchangeStr == "COINBASE") {
                    price = CoinbaseMdMgr::GetInstance().GetAssetPrice(baseAsset);
                }
                
                if (price > MINDOUBLE) {
                    yNetValue += iter->second.initialAmount / price;
                    netValueS += iter->second.netAmountS / price;
                    netValueD += iter->second.netAmountD / price;
                    if (iter->second.netAmountD > 0) {
                        totalAvailMargin += iter->second.netAmountD / price * assetRate;
                    }
                    totalPositionValue += iter->second.positionValueD / price;
                }
            } else {
                string key = exchangeStr + "|" + asset + "-" + baseAsset + "|SPOT";  // 首先获取币对现货价格
                double price = 0.0;
                if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                    price = BinanceMdMgr::GetInstance().GetMidPrice(key);
                } else if (exchangeStr == "COINBASE") {
                    price = CoinbaseMdMgr::GetInstance().GetMidPrice(key);
                }
                
                if (price > MINDOUBLE) {
                    yNetValue += iter->second.initialAmount * price;
                    netValueS += iter->second.netAmountS * price;
                    netValueD += iter->second.netAmountD * price;
                    if (iter->second.netAmountD > 0) {
                        totalAvailMargin += iter->second.netAmountD * price * assetRate;
                    }
                    totalPositionValue += iter->second.positionValueD / price;
                } else {
                    if (baseAsset == "USDT" || baseAsset == "USD") {
                        double price = 0.0;
                        if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                            price = BinanceMdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                        } else if (exchangeStr == "COINBASE") {
                            price = CoinbaseMdMgr::GetInstance().GetAssetPrice(asset);
                        }
                        
                        yNetValue += iter->second.initialAmount * price;
                        netValueS += iter->second.netAmountS * price;
                        netValueD += iter->second.netAmountD * price;
                        if (iter->second.netAmountD > 0) {
                            totalAvailMargin += iter->second.netAmountD * price * assetRate;
                        }
                        totalPositionValue += iter->second.positionValueD * price;
                    } else {
                        double priceU = 0.0;
                        double priceBaseAsset = 0.0;
                        if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                            priceU = BinanceMdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                            priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                        } else if (exchangeStr == "COINBASE") {
                            priceU = CoinbaseMdMgr::GetInstance().GetAssetPrice(asset);
                            priceBaseAsset = CoinbaseMdMgr::GetInstance().GetAssetPrice(baseAsset);
                        }

                        if (priceBaseAsset > MINDOUBLE) {
                            yNetValue += iter->second.initialAmount * priceU / priceBaseAsset;
                            netValueS += iter->second.netAmountS * priceU / priceBaseAsset;
                            netValueD += iter->second.netAmountD * priceU / priceBaseAsset;
                            if (iter->second.netAmountD > 0) {
                                totalAvailMargin += iter->second.netAmountD * priceU / priceBaseAsset * assetRate;
                            }
                            totalPositionValue += iter->second.positionValueD * priceU / priceBaseAsset;
                        }
                    }
                }
            }
        }
    }
    riskInfo.netValueS = netValueS;
    riskInfo.netValueD = netValueD;
    riskInfo.yNetValue = yNetValue;

    if (fabs(riskInfo.yNetValue) > MINDOUBLE) {
        riskInfo.netValueGrowthRateS = riskInfo.netValueS / riskInfo.yNetValue - 1;   // 计算方式不对  
    }
    riskInfo.netValueGrowthRateD = 0.0;

    double maxRealLeverageS = 0.0;
    double maxRealLeverageD = 0.0;
    for (auto iter = mCFutureAsset.begin(); iter != mCFutureAsset.end(); ++iter) {
        if (iter->second.realLeverageRatioS > maxRealLeverageS) {
            maxRealLeverageS = iter->second.realLeverageRatioS;
            maxLeverageTabS = "CFuture";
            maxLeverageUS = true;
            maxLeverageAssetS = iter->first;
        }
        if (iter->second.realLeverageRatioD > maxRealLeverageD) {
            maxRealLeverageD = iter->second.realLeverageRatioD;
            maxLeverageTabD = "CFuture";
            maxLeverageUD = true;
            maxLeverageAssetD = iter->first;
        }
    }

    for (auto iter = mUFutureAsset.begin(); iter != mUFutureAsset.end(); ++iter) {
        if (iter->second.realLeverageRatioS > maxRealLeverageS) {
            maxRealLeverageS = iter->second.realLeverageRatioS;
            maxLeverageTabS = "UFuture";
            maxLeverageUS = true;
            maxLeverageAssetS = iter->first;
        }
        if (iter->second.realLeverageRatioD > maxRealLeverageD) {
            maxRealLeverageD = iter->second.realLeverageRatioD;
            maxLeverageTabD = "UFuture";
            maxLeverageUD = true;
            maxLeverageAssetD = iter->first;
        }
    }

    for (auto iter = mDeliveryAsset.begin(); iter != mDeliveryAsset.end(); ++iter) {
        if (iter->second.realLeverageRatioS > maxRealLeverageS) {
            maxRealLeverageS = iter->second.realLeverageRatioS;
            maxLeverageTabS = "Delivery";
            maxLeverageUS = true;
            maxLeverageAssetS = iter->first;
        }
        if (iter->second.realLeverageRatioD > maxRealLeverageD) {
            maxRealLeverageD = iter->second.realLeverageRatioD;
            maxLeverageTabD = "Delivery";
            maxLeverageUD = true;
            maxLeverageAssetD = iter->first;
        }
    }

    for (auto iter = mPerpetualAsset.begin(); iter != mPerpetualAsset.end(); ++iter) {
        if (iter->second.realLeverageRatioS > maxRealLeverageS) {
            maxRealLeverageS = iter->second.realLeverageRatioS;
            // maxLeverageTabS = "Delivery";
            maxLeverageTabS = "Perpetual";
            maxLeverageUS = true;
            maxLeverageAssetS = iter->first;
        }
        if (iter->second.realLeverageRatioD > maxRealLeverageD) {
            maxRealLeverageD = iter->second.realLeverageRatioD;
            // maxLeverageTabD = "Delivery";
            maxLeverageTabD = "Perpetual";
            maxLeverageUD = true;
            maxLeverageAssetD = iter->first;
        }
    }

    riskInfo.maxRealLeverageS = maxRealLeverageS;
    riskInfo.maxRealLeverageD = maxRealLeverageD;

    if (unified == 1) {
        if (totalMarginBalance > 0) {
            riskInfo.maxRealLeverageD = totalPositionValue / totalMarginBalance;
        } else {
            if (totalAvailMargin > 0) {
                riskInfo.maxRealLeverageD = totalPositionValue / totalAvailMargin;
            }
        }
    }

    if (exchangeStr == "BYBIT") {
        BybitAdapterItem* item = BybitAdapterMgr::GetInstance().GetAdapterItem(customerId);
        if (item) {
            bybit::TotalAccountInfo& totalAccountInfo = item->GetTotalAccountInfo();
            double accountRate = totalAccountInfo.accountMMRate;
            double rate = totalPositionValue / totalMarginBalance;
            LOG_INFO("BYBIT accountInfo: accountId:%d accountRate:%f  rate:%f", customerId, accountRate, rate);
        }
    }

    double riskExposureS = 0.0;
    double riskExposureD = 0.0;
    unordered_map<string, igmonitor::Exposure> mExposureUnion;
    for (auto iter = mExposure.begin(); iter != mExposure.end(); ++iter) {
        if (iter->first == baseAsset) {
            continue;
        }

        if (baseAsset == "USDT") {
            if (iter->first == "BUSD") {
                continue;
            }
        }

        if (iter->first == "USDT") {
            auto it = mExposureUnion.find(iter->first);
            if (it != mExposureUnion.end()) {
                it->second.exposureValueD += iter->second.exposureValueD;
            } else {
                mExposureUnion[iter->first] = iter->second;
            }
        } else if (iter->first == "BUSD") {
            auto it = mExposureUnion.find("USDT");
            if (it != mExposureUnion.end()) {
                it->second.exposureValueD += iter->second.exposureValueD;
            } else {
                mExposureUnion["USDT"] = iter->second;
            }
        } else {
            mExposureUnion[iter->first] = iter->second;
        }
    }

    for (auto iter = mExposureUnion.begin(); iter != mExposureUnion.end(); ++iter) {
        if (fabs(iter->second.exposureValueD) > fabs(riskExposureD)) {
            riskExposureD = iter->second.exposureValueD;
            maxRiskExposureAssetD = iter->first;
        }
        if (fabs(iter->second.exposureValueS) > fabs(riskExposureS)) {
            riskExposureS = iter->second.exposureValueS;
            maxRiskExposureAssetS = iter->first;
        }

        // calculate total exposure
        double exposure = iter->second.exposureValueD;
        /*
	if (baseAsset != "USDT" && baseAsset != "USD") {
            double price = 0.0;
            if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO") {
                price = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
            } else if (exchangeStr == "COINBASE") {
                price = CoinbaseMdMgr::GetInstance().GetAssetPrice(baseAsset);
            }
            exposure *= price;
        }
	*/
        totalExposure += fabs(exposure);

    }

    riskInfo.riskExposureS = riskExposureS;
    riskInfo.riskExposureD = riskExposureD;

    double underwayOrderValueS = 0.0;
    double underwayOrderValueD = 0.0;

    for (auto iter = mUFuturePosition.begin(); iter != mUFuturePosition.end(); ++iter) {
        string key = iter->second.key;
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        double underwayOrderValue = 0.0;
        double underwayOrderValued = 0.0;
        if (baseAsset == "USDT") {
            underwayOrderValue = iter->second.underwayAbsPositionS * price *  info.multiple;
            underwayOrderValued = iter->second.underwayAbsPositioD * price * info.multiple;
        } else {
            double priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
            if (priceBaseAsset > MINDOUBLE) {
                underwayOrderValue = iter->second.underwayAbsPositionS * price * info.multiple / priceBaseAsset;
                underwayOrderValued = iter->second.underwayAbsPositioD * price * info.multiple / priceBaseAsset;
            }
        }
        
        if (underwayOrderValue > underwayOrderValueS) {
            underwayOrderValueS = underwayOrderValue;
        }
        if (underwayOrderValued > underwayOrderValueD) {
            //underwayOrderValueD = underwayOrderValued;
        }
    }

    for (auto iter = mCFuturePosition.begin(); iter != mCFuturePosition.end(); ++iter) {
        string key = iter->second.key;
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        double underwayOrderValue = 0.0;
        double underwayOrderValued = 0.0;
        if (baseAsset == "USDT") {
            underwayOrderValue = iter->second.underwayAbsPositionS * info.multiple;
            underwayOrderValued = iter->second.underwayAbsPositioD * info.multiple;
        } else {
            if (baseAsset == info.margin) {
                if (price > MINDOUBLE) {
                    underwayOrderValue = iter->second.underwayAbsPositionS * info.multiple / price;
                    underwayOrderValued = iter->second.underwayAbsPositioD * info.multiple / price;
                }
            } else {
                double priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                if (priceBaseAsset > MINDOUBLE) {
                    underwayOrderValue = iter->second.underwayAbsPositionS * info.multiple / priceBaseAsset;
                    underwayOrderValued = iter->second.underwayAbsPositioD * info.multiple / priceBaseAsset;
                }
            }
        }
        if (underwayOrderValue > underwayOrderValueS) {
            underwayOrderValueS = underwayOrderValue;
        }
        if (underwayOrderValued > underwayOrderValueD) {
            //underwayOrderValueD = underwayOrderValued;
        }
    }
    riskInfo.underwayOrderValueS = underwayOrderValueS;

    
    for (auto iter = mSpotAsset.begin(); iter != mSpotAsset.end(); ++iter) {
        if (iter->second.underwayOrderValueD > underwayOrderValueD) {
            underwayOrderValueD = iter->second.underwayOrderValueD;
        }
    }
/*
    for (auto iter = mUFutureAsset.begin(); iter != mUFutureAsset.end(); ++iter) {
        if (iter->second.underwayOrderValueD > underwayOrderValueD) {
            underwayOrderValueD = iter->second.underwayOrderValueD;
        }
    }

    for (auto iter = mCFutureAsset.begin(); iter != mCFutureAsset.end(); ++iter) {
        if (iter->second.underwayOrderValueD > underwayOrderValueD) {
            underwayOrderValueD = iter->second.underwayOrderValueD;
        }   
    }
*/
    

/*
    for (auto iter = mSpotOpenOrder.begin(); iter != mSpotOpenOrder.end(); ++iter) {
        double underwayOrderValue = 0.0;
        string key = iter->first;
        InstrumentInfo info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (info.instLeft == baseAsset) {
            underwayOrderValue = iter->second.openQty;
        } else {
            double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
            if (price > MINDOUBLE) {
                underwayOrderValue = iter->second.openQty * price;
            } else {
                if (baseAsset == "USDT") {
                    double price = BinanceMdMgr::GetInstance().GetAssetPrice(info.instLeft);
                    if (price > MINDOUBLE) {
                        underwayOrderValue = iter->second.openQty * price;
                    }
                } else {
                    double priceU = BinanceMdMgr::GetInstance().GetAssetPrice(info.instLeft);
                    double priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset);
                    if (priceBaseAsset > MINDOUBLE) {
                        underwayOrderValue = iter->second.openQty * priceU / priceBaseAsset;
                    }
                }
            }
        }

        if (underwayOrderValue > underwayOrderValueD) {
            underwayOrderValueD = underwayOrderValue;
        }
    }
*/
    for (auto iter = mUFutureOpenOrder.begin(); iter != mUFutureOpenOrder.end(); ++iter) {
        double underwayOrderValue = 0.0;
        string key = iter->first;
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (baseAsset == "USDT") {
            underwayOrderValue = iter->second.openQty * price * info.multiple;
        } else {
            double priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
            if (priceBaseAsset > MINDOUBLE) {
                underwayOrderValue = iter->second.openQty * price * info.multiple / priceBaseAsset;
            }
        }
        if (underwayOrderValue > underwayOrderValueD) {
            underwayOrderValueD = underwayOrderValue;
        }
        LOG_INFO("underwayOrderValue: accountId: %d  underwayOrderValue: %f", customerId, underwayOrderValue); 
    }

    for (auto iter = mCFutureOpenOrder.begin(); iter != mCFutureOpenOrder.end(); ++iter) {
        string key = iter->first;
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        double underwayOrderValue = 0.0;
        if (baseAsset == "USDT") {
            underwayOrderValue = iter->second.openQty * info.multiple;
        } else {
            if (baseAsset == info.margin) {
                if (price > MINDOUBLE) {
                    underwayOrderValue = iter->second.openQty * info.multiple / price;
                }
            } else {
                double priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                if (priceBaseAsset > MINDOUBLE) {
                    underwayOrderValue = iter->second.openQty * info.multiple / priceBaseAsset;
                }
            }
        }
        if (underwayOrderValue > underwayOrderValueD) {
            underwayOrderValueD = underwayOrderValue;
        }
        LOG_INFO("underwayOrderValue: accountId: %d  underwayOrderValue: %f", customerId, underwayOrderValue); 
    }

    for (auto iter = mDeliveryOpenOrder.begin(); iter != mDeliveryOpenOrder.end(); ++iter) {
        double underwayOrderValue = 0.0;
        string key = iter->first;
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (info.instrumentType == "InstType_USDT_FUTURES" || info.calculateType == 0) {
            if (baseAsset == "USDT") {
                underwayOrderValue = iter->second.openQty * price * info.multiple;
            } else {
                double priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                if (priceBaseAsset > MINDOUBLE) {
                    underwayOrderValue = iter->second.openQty * price * info.multiple / priceBaseAsset;
                }
            }
        } else if (info.instrumentType == "InstType_BTC_FUTURES" || info.calculateType == 1) {
            if (baseAsset == "USDT") {
                underwayOrderValue = iter->second.openQty * info.multiple;
            } else {
                if (baseAsset == info.margin) {
                    if (price > MINDOUBLE) {
                        underwayOrderValue = iter->second.openQty * info.multiple / price;
                    }
                } else {
                    double priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                    if (priceBaseAsset > MINDOUBLE) {
                        underwayOrderValue = iter->second.openQty * info.multiple / priceBaseAsset;
                    }
                }
            }
        }

        if (underwayOrderValue > underwayOrderValueD) {
            underwayOrderValueD = underwayOrderValue;
        }
        LOG_INFO("underwayOrderValue: accountId: %d  underwayOrderValue: %f", customerId, underwayOrderValue); 
    }

    for (auto iter = mPerpetualOpenOrder.begin(); iter != mPerpetualOpenOrder.end(); ++iter) {
        double underwayOrderValue = 0.0;
        string key = iter->first;
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (info.instrumentType == "InstType_USDT_SWAP" || info.calculateType == 0) {
            if (baseAsset == "USDT") {
                underwayOrderValue = iter->second.openQty * price * info.multiple;
            } else {
                double priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                if (priceBaseAsset > MINDOUBLE) {
                    underwayOrderValue = iter->second.openQty * price * info.multiple / priceBaseAsset;
                }
            }
        } else if (info.instrumentType == "InstType_BTC_SWAP" || info.calculateType == 1) {
            if (baseAsset == "USDT") {
                underwayOrderValue = iter->second.openQty * info.multiple;
            } else {
                if (baseAsset == info.margin) {
                    if (price > MINDOUBLE) {
                        underwayOrderValue = iter->second.openQty * info.multiple / price;
                    }
                } else {
                    double priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                    if (priceBaseAsset > MINDOUBLE) {
                        underwayOrderValue = iter->second.openQty * info.multiple / priceBaseAsset;
                    }
                }
            }
        }

        if (underwayOrderValue > underwayOrderValueD) {
            underwayOrderValueD = underwayOrderValue;
        }
        LOG_INFO("underwayOrderValue: accountId: %d  underwayOrderValue: %f", customerId, underwayOrderValue); 
    }

    riskInfo.underwayOrderValueD = underwayOrderValueD;

    LOG_INFO("riskInfo: %s", riskInfo.toString().c_str());  
}

string BinanceAccountItem::GetAccountType() {
    return type;
}

int BinanceAccountItem::GetAccountId() {
    return customerId;
}

string BinanceAccountItem::GetExchangeStr() {
    return exchangeStr;
}

igmonitor::RiskInfo& BinanceAccountItem::GetRiskInfo() {
    return riskInfo;
}

unordered_map<string, igmonitor::Exposure>& BinanceAccountItem::GetExposure() {
    return mExposure;
}

unordered_map<string, igmonitor::Asset>& BinanceAccountItem::GetSpotAsset() {
    return mSpotAsset;
}

unordered_map<string, igmonitor::Asset>& BinanceAccountItem::GetUFutureAsset() {
    return mUFutureAsset;
}

unordered_map<string, igmonitor::Asset>& BinanceAccountItem::GetCFutureAsset() {
    return mCFutureAsset;
}

unordered_map<string, igmonitor::Asset>& BinanceAccountItem::GetTotalAsset() {
    return mTotalAsset;
}

unordered_map<string, igmonitor::Position>& BinanceAccountItem::GetUFuturePosition() {
    return mUFuturePosition;
}

unordered_map<string, igmonitor::Position>& BinanceAccountItem::GetCFuturePosition() {
    return mCFuturePosition;
}

unordered_map<string, igmonitor::PositionFundingRate>& BinanceAccountItem::GetPositionFundingRate() {
    return mPositionFundingRate;
}

set<string> BinanceAccountItem::GetInstrumentList() {
    set<string> s;
/*
    if (baseAsset != "USDT") {
        string instrumentKey = "BINANCE|" + baseAsset + "USDT" + "|SPOT";
        s.insert(instrumentKey);
        instrumentKey = "BINANCE|" + baseAsset + "USDT" + "|SWAP";
        s.insert(instrumentKey);
    }

        for (auto iter = mSpotAsset.begin(); iter != mSpotAsset.end(); ++iter) {
            string asset = iter->first;
            if (asset != baseAsset && asset != "USDT") {
                string instrumentKey = "BINANCE|" + asset + baseAsset + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = "BINANCE|" + asset + "USDT" + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = "BINANCE|" + asset + "USDT" + "|SWAP";
                s.insert(instrumentKey);
            }
        }


    map<string, Asset> mUFutureAssetTemp = mUFutureAsset;
    if (mUFutureAssetTemp.size() > 0) {
        for (auto iter = mUFutureAssetTemp.begin(); iter != mUFutureAssetTemp.end(); ++iter) {
            string asset = iter->first;
            if (asset != baseAsset && asset != "USDT") {
                string instrumentKey = "BINANCE|" + asset + baseAsset + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = "BINANCE|" + asset + "USDT" + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = "BINANCE|" + asset + "USDT" + "|SWAP";
                s.insert(instrumentKey);
            }
        }
    }

    map<string, Asset> mCFutureAssetTemp = mCFutureAsset;
    if (mCFutureAssetTemp.size() > 0) {
        for (auto iter = mCFutureAssetTemp.begin(); iter != mCFutureAssetTemp.end(); ++iter) {
            string asset = iter->first;
            if (asset != baseAsset && asset != "USDT") {
                string instrumentKey = "BINANCE|" + asset + baseAsset + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = "BINANCE|" + asset + "USDT" + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = "BINANCE|" + asset + "USDT" + "|SWAP";
                s.insert(instrumentKey);
            }
        }
    }

    map<string, Position> mUFuturePositionTemp = mUFuturePosition;
    if (mUFuturePositionTemp.size() > 0) {
        for (auto iter = mUFuturePositionTemp.begin(); iter != mUFuturePositionTemp.end(); ++iter) {
            string instrumentKey = "BINANCE|" + iter->first + "|FUTURES";
            s.insert(instrumentKey);

            string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
            InstrumentInfo info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
            instrumentKey = "BINANCE|" + info.instLeft + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "BINANCE|" + info.instLeft + "USDT" + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "BINANCE|" + info.instLeft + "USDT" + "|SWAP";
            s.insert(instrumentKey);
        }
    }

    map<string, Position> mCFuturePositionTemp = mCFuturePosition;
    if (mCFuturePositionTemp.size() > 0) {
        for (auto iter = mCFuturePositionTemp.begin(); iter != mCFuturePositionTemp.end(); ++iter) {
            string instrumentKey = "BINANCE|" + iter->first + "|FUTURES";
            s.insert(instrumentKey);
        }
    }

    map<string, SavAsset> mSavAssetTemp = mSavAsset;
    if (mSavAssetTemp.size() > 0) {
        for (auto iter = mSavAssetTemp.begin(); iter != mSavAssetTemp.end(); ++iter) {
            string asset = iter->first;
            if (asset != baseAsset && asset != "USDT") {
                string instrumentKey = "BINANCE|" + asset + baseAsset + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = "BINANCE|" + asset + "USDT" + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = "BINANCE|" + asset + "USDT" + "|SWAP";
                s.insert(instrumentKey);
            }
        }
    }

    map<string, OpenOrder> mSpotOpenOrderTemp = mSpotOpenOrder;
    if (mSpotOpenOrderTemp.size() > 0) {
        for (auto iter = mSpotOpenOrderTemp.begin(); iter != mSpotOpenOrderTemp.end(); ++iter) {
            string symbol = iter->second.symbol;
            string instrumentKey = "BINANCE|" + symbol + "|SPOT";
            s.insert(instrumentKey);
        }
    }

    map<string, OpenOrder> mUFutureOpenOrderTemp = mUFutureOpenOrder;
    if (mUFutureOpenOrderTemp.size() > 0) {
        for (auto iter = mUFutureOpenOrderTemp.begin(); iter != mUFutureOpenOrderTemp.end(); ++iter) {
            string symbol = iter->second.symbol;
            string instrumentKey = "BINANCE|" + symbol + "|FUTURES";
            s.insert(instrumentKey);
        }
    }

    map<string, OpenOrder> mCFutureOpenOrderTemp = mCFutureOpenOrder;
    if (mCFutureOpenOrderTemp.size() > 0) {
        for (auto iter = mCFutureOpenOrderTemp.begin(); iter != mCFutureOpenOrderTemp.end(); ++iter) {
            string symbol = iter->second.symbol;
            string instrumentKey = "BINANCE|" + symbol + "|FUTURES";
            s.insert(instrumentKey);
        }
    }
*/
    return s;
}

web::json::value BinanceAccountItem::GetDetail() {
    web::json::value detailV;
    detailV["customer_id"] = web::json::value::number(customerId);
    web::json::value totalAssetV;

    for (auto iter = mTotalAsset.begin(); iter != mTotalAsset.end(); ++iter) {
        web::json::value assetV;
        char data[24];
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV["asset"] = web::json::value::string(data);
        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.realLeverageRatioD);
            assetV["realLeverageRatio"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.realLeverageRatioS);
            assetV["realLeverageRatio"] = web::json::value::string(data);
        }

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.positionValueD);
            assetV["positionValue"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.positionValueS);
            assetV["positionValue"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV["transferAmount"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV["transferFrozenAmount"] = web::json::value::string(data);

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.netAmountD);
            assetV["netAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.netAmountS);
            assetV["netAmount"] = web::json::value::string(data);
        }
        
        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.availableAmountD);
            assetV["availableAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.availableAmountS);
            assetV["availableAmount"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.totalAmountS);
        assetV["totalAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV["totalAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountS);
        assetV["frozenAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV["frozenAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountS);
        assetV["floatAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV["floatAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountS);
        assetV["marginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV["marginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountS);
        assetV["frozenMarginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV["frozenMarginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV["initialAmount"] = web::json::value::string(data);

        if (type == "physical") {
            if (fabs(iter->second.positionValueS) >= MINDOUBLE) {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) / fabs(iter->second.positionValueS) > 0.01) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.totalAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.totalAmountS / iter->second.totalAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.totalAmountS - iter->second.totalAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.frozenAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.frozenAmountS / iter->second.frozenAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.frozenAmountS - iter->second.frozenAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            }

            if (iter->second.marginAmountS < iter->second.marginAmountD) {
                sprintf(data, "%f|1", iter->second.marginAmountS);
                assetV["marginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.marginAmountD);
                assetV["marginAmount_d"] = web::json::value::string(data);
            }

            if (iter->second.frozenMarginAmountS < iter->second.frozenMarginAmountD) {
                sprintf(data, "%f|1", iter->second.frozenMarginAmountS);
                assetV["frozenMarginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.frozenMarginAmountD);
                assetV["frozenMarginAmount_d"] = web::json::value::string(data);
            }
        }
        totalAssetV[iter->second.asset] = assetV;
    }

    if (totalAssetV.size() == 0) {
        detailV["TotalAssetChecked"] = web::json::value::object();
    } else {
        detailV["TotalAssetChecked"] = totalAssetV;
    }


    web::json::value spotAssetV;
    for (auto iter = mSpotAsset.begin(); iter != mSpotAsset.end(); ++iter) {
        web::json::value assetV;
        char data[24];
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV["asset"] = web::json::value::string(data);
        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.realLeverageRatioD);
            assetV["realLeverageRatio"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.realLeverageRatioS);
            assetV["realLeverageRatio"] = web::json::value::string(data);
        }
        
        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.positionValueD);
            assetV["positionValue"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.positionValueS);
            assetV["positionValue"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV["transferAmount"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV["transferFrozenAmount"] = web::json::value::string(data);

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.netAmountD);
            assetV["netAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.netAmountS);
            assetV["netAmount"] = web::json::value::string(data);
        }
        
        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.availableAmountD);
            assetV["availableAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.availableAmountS);
            assetV["availableAmount"] = web::json::value::string(data);
        }
        
        sprintf(data, "%f|0", iter->second.floatAmountS);
        assetV["floatAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV["floatAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV["initialAmount"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.totalAmountS);
        assetV["totalAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV["totalAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountS);
        assetV["frozenAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV["frozenAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountS);
        assetV["marginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV["marginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountS);
        assetV["frozenMarginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV["frozenMarginAmount_d"] = web::json::value::string(data);

        if (type == "physical") {
            if (fabs(iter->second.positionValueS) >= MINDOUBLE) {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) / fabs(iter->second.positionValueS) > 0.01) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.totalAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.totalAmountS / iter->second.totalAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.totalAmountS - iter->second.totalAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.frozenAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.frozenAmountS / iter->second.frozenAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.frozenAmountS - iter->second.frozenAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            }

            if (iter->second.marginAmountS < iter->second.marginAmountD) {
                sprintf(data, "%f|1", iter->second.marginAmountS);
                assetV["marginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.marginAmountD);
                assetV["marginAmount_d"] = web::json::value::string(data);
            }

            if (iter->second.frozenMarginAmountS < iter->second.frozenMarginAmountD) {
                sprintf(data, "%f|1", iter->second.frozenMarginAmountS);
                assetV["frozenMarginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.frozenMarginAmountD);
                assetV["frozenMarginAmount_d"] = web::json::value::string(data);
            }
        }
        spotAssetV[iter->second.asset] = assetV;
    }
 
    if (spotAssetV.size() == 0) {
        detailV["SpotAssetChecked"] = web::json::value::object();
    } else {
        detailV["SpotAssetChecked"] = spotAssetV;
    }


    web::json::value uFutureAssetV;
    for (auto iter = mUFutureAsset.begin(); iter != mUFutureAsset.end(); ++iter) {
        web::json::value assetV;
        char data[48];
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV["asset"] = web::json::value::string(data);
        if (type == "physical") {
            if (maxLeverageUD == true && maxLeverageAssetD == iter->first) {
                sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioD);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            } else {
                sprintf(data, "%f|0", iter->second.realLeverageRatioD);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            }
        } else {
            if (maxLeverageUS == true && maxLeverageAssetS == iter->first) {
                sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioS);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            } else {
                sprintf(data, "%f|0", iter->second.realLeverageRatioS);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            }
        }
        
        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.positionValueD);
            assetV["positionValue"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.positionValueS);
            assetV["positionValue"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV["transferAmount"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV["transferFrozenAmount"] = web::json::value::string(data);

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.netAmountD);
            assetV["netAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.netAmountS);
            assetV["netAmount"] = web::json::value::string(data);
        }

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.availableAmountD);
            assetV["availableAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.availableAmountS);
            assetV["availableAmount"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.totalAmountS);
        assetV["totalAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV["totalAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountS);
        assetV["frozenAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV["frozenAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountS);
        assetV["floatAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV["floatAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountS);
        assetV["marginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV["marginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountS);
        assetV["frozenMarginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV["frozenMarginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV["initialAmount"] = web::json::value::string(data);

        if (type == "physical") {
            if (fabs(iter->second.positionValueS) >= MINDOUBLE) {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) / fabs(iter->second.positionValueS) > 0.01) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.totalAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.totalAmountS / iter->second.totalAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.totalAmountS - iter->second.totalAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.frozenAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.frozenAmountS / iter->second.frozenAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.frozenAmountS - iter->second.frozenAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            }

            if (iter->second.marginAmountS < iter->second.marginAmountD) {
                sprintf(data, "%f|1", iter->second.marginAmountS);
                assetV["marginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.marginAmountD);
                assetV["marginAmount_d"] = web::json::value::string(data);
            }

            if (iter->second.frozenMarginAmountS < iter->second.frozenMarginAmountD) {
                sprintf(data, "%f|1", iter->second.frozenMarginAmountS);
                assetV["frozenMarginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.frozenMarginAmountD);
                assetV["frozenMarginAmount_d"] = web::json::value::string(data);
            }
        }
        uFutureAssetV[iter->second.asset] = assetV;
    }

    if (uFutureAssetV.size() == 0) {
        detailV["UFutureAssetChecked"] = web::json::value::object();
    } else {
        detailV["UFutureAssetChecked"] = uFutureAssetV;
    }


    web::json::value cFutureAssetV;
    for (auto iter = mCFutureAsset.begin(); iter != mCFutureAsset.end(); ++iter) {
        web::json::value assetV;
        char data[48];
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV["asset"] = web::json::value::string(data);

        if (type == "physical") {
            if (maxLeverageUD == false && maxLeverageAssetD == iter->first) {
                sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioD);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            } else {
                sprintf(data, "%f|0", iter->second.realLeverageRatioD);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            }
        } else {
            if (maxLeverageUS == false && maxLeverageAssetS == iter->first) {
                sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioS);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            } else {
                sprintf(data, "%f|0", iter->second.realLeverageRatioS);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            }
        }
        
        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.positionValueD);
            assetV["positionValue"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.positionValueS);
            assetV["positionValue"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV["transferAmount"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV["transferFrozenAmount"] = web::json::value::string(data);

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.netAmountD);
            assetV["netAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.netAmountS);
            assetV["netAmount"] = web::json::value::string(data);
        }

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.availableAmountD);
            assetV["availableAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.availableAmountS);
            assetV["availableAmount"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.totalAmountS);
        assetV["totalAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV["totalAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountS);
        assetV["frozenAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV["frozenAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountS);
        assetV["floatAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV["floatAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountS);
        assetV["marginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV["marginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountS);
        assetV["frozenMarginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV["frozenMarginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV["initialAmount"] = web::json::value::string(data);

        if (type == "physical") {
            if (fabs(iter->second.positionValueS) >= MINDOUBLE) {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) / fabs(iter->second.positionValueS) > 0.01) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.totalAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.totalAmountS / iter->second.totalAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.totalAmountS - iter->second.totalAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.frozenAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.frozenAmountS / iter->second.frozenAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.frozenAmountS - iter->second.frozenAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            }

            if (iter->second.marginAmountS < iter->second.marginAmountD) {
                sprintf(data, "%f|1", iter->second.marginAmountS);
                assetV["marginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.marginAmountD);
                assetV["marginAmount_d"] = web::json::value::string(data);
            }

            if (iter->second.frozenMarginAmountS < iter->second.frozenMarginAmountD) {
                sprintf(data, "%f|1", iter->second.frozenMarginAmountS);
                assetV["frozenMarginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.frozenMarginAmountD);
                assetV["frozenMarginAmount_d"] = web::json::value::string(data);
            }
        }

        cFutureAssetV[iter->second.asset] = assetV;
    }

    if (cFutureAssetV.size() == 0) {
        detailV["CFutureAssetChecked"] = web::json::value::object();
    } else {
        detailV["CFutureAssetChecked"] = cFutureAssetV;
    }

    // delivery asset
    web::json::value deliveryAssetV;
    for (auto iter = mDeliveryAsset.begin(); iter != mDeliveryAsset.end(); ++iter) {
        web::json::value assetV;
        char data[48];
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV["asset"] = web::json::value::string(data);

        if (type == "physical") {
            if (maxLeverageUD == false && maxLeverageAssetD == iter->first) {
                sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioD);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            } else {
                sprintf(data, "%f|0", iter->second.realLeverageRatioD);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            }
        } else {
            if (maxLeverageUS == false && maxLeverageAssetS == iter->first) {
                sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioS);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            } else {
                sprintf(data, "%f|0", iter->second.realLeverageRatioS);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            }
        }
        
        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.positionValueD);
            assetV["positionValue"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.positionValueS);
            assetV["positionValue"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV["transferAmount"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV["transferFrozenAmount"] = web::json::value::string(data);

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.netAmountD);
            assetV["netAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.netAmountS);
            assetV["netAmount"] = web::json::value::string(data);
        }

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.availableAmountD);
            assetV["availableAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.availableAmountS);
            assetV["availableAmount"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.totalAmountS);
        assetV["totalAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV["totalAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountS);
        assetV["frozenAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV["frozenAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountS);
        assetV["floatAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV["floatAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountS);
        assetV["marginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV["marginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountS);
        assetV["frozenMarginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV["frozenMarginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV["initialAmount"] = web::json::value::string(data);

        if (type == "physical") {
            if (fabs(iter->second.positionValueS) >= MINDOUBLE) {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) / fabs(iter->second.positionValueS) > 0.01) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.totalAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.totalAmountS / iter->second.totalAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.totalAmountS - iter->second.totalAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.frozenAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.frozenAmountS / iter->second.frozenAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.frozenAmountS - iter->second.frozenAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            }

            if (iter->second.marginAmountS < iter->second.marginAmountD) {
                sprintf(data, "%f|1", iter->second.marginAmountS);
                assetV["marginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.marginAmountD);
                assetV["marginAmount_d"] = web::json::value::string(data);
            }

            if (iter->second.frozenMarginAmountS < iter->second.frozenMarginAmountD) {
                sprintf(data, "%f|1", iter->second.frozenMarginAmountS);
                assetV["frozenMarginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.frozenMarginAmountD);
                assetV["frozenMarginAmount_d"] = web::json::value::string(data);
            }
        }

        deliveryAssetV[iter->second.asset] = assetV;
    }

    if (deliveryAssetV.size() == 0) {
        detailV["DeliveryAssetChecked"] = web::json::value::object();
    } else {
        detailV["DeliveryAssetChecked"] = deliveryAssetV;
    }

    // perpetual asset
    web::json::value perpetualAssetV;
    for (auto iter = mPerpetualAsset.begin(); iter != mPerpetualAsset.end(); ++iter) {
        web::json::value assetV;
        char data[48];
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV["asset"] = web::json::value::string(data);

        if (type == "physical") {
            if (maxLeverageUD == false && maxLeverageAssetD == iter->first) {
                sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioD);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            } else {
                sprintf(data, "%f|0", iter->second.realLeverageRatioD);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            }
        } else {
            if (maxLeverageUS == false && maxLeverageAssetS == iter->first) {
                sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioS);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            } else {
                sprintf(data, "%f|0", iter->second.realLeverageRatioS);
                assetV["realLeverageRatio"] = web::json::value::string(data);
            }
        }
        
        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.positionValueD);
            assetV["positionValue"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.positionValueS);
            assetV["positionValue"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV["transferAmount"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV["transferFrozenAmount"] = web::json::value::string(data);

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.netAmountD);
            assetV["netAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.netAmountS);
            assetV["netAmount"] = web::json::value::string(data);
        }

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.availableAmountD);
            assetV["availableAmount"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.availableAmountS);
            assetV["availableAmount"] = web::json::value::string(data);
        }

        sprintf(data, "%f|0", iter->second.totalAmountS);
        assetV["totalAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV["totalAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountS);
        assetV["frozenAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV["frozenAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountS);
        assetV["floatAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV["floatAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountS);
        assetV["marginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV["marginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountS);
        assetV["frozenMarginAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV["frozenMarginAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV["initialAmount"] = web::json::value::string(data);

        if (type == "physical") {
            if (fabs(iter->second.positionValueS) >= MINDOUBLE) {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) / fabs(iter->second.positionValueS) > 0.01) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    assetV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    assetV["floatAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.totalAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.totalAmountS / iter->second.totalAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.totalAmountS - iter->second.totalAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.totalAmountS);
                    assetV["totalAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.totalAmountD);
                    assetV["totalAmount_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.frozenAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.frozenAmountS / iter->second.frozenAmountD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.frozenAmountS - iter->second.frozenAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.frozenAmountS);
                    assetV["frozenAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.frozenAmountD);
                    assetV["frozenAmount_d"] = web::json::value::string(data);
                }
            }

            if (iter->second.marginAmountS < iter->second.marginAmountD) {
                sprintf(data, "%f|1", iter->second.marginAmountS);
                assetV["marginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.marginAmountD);
                assetV["marginAmount_d"] = web::json::value::string(data);
            }

            if (iter->second.frozenMarginAmountS < iter->second.frozenMarginAmountD) {
                sprintf(data, "%f|1", iter->second.frozenMarginAmountS);
                assetV["frozenMarginAmount_s"] = web::json::value::string(data);
                sprintf(data, "%f|1", iter->second.frozenMarginAmountD);
                assetV["frozenMarginAmount_d"] = web::json::value::string(data);
            }
        }

        perpetualAssetV[iter->second.asset] = assetV;
    }

    if (perpetualAssetV.size() == 0) {
        detailV["PerpetualAssetChecked"] = web::json::value::object();
    } else {
        detailV["PerpetualAssetChecked"] = perpetualAssetV;
    }


    web::json::value uFuturePositionV;
    for (auto iter = mUFuturePosition.begin(); iter != mUFuturePosition.end(); ++iter) {
        web::json::value positionV;
        char data[24];
        sprintf(data, "%s", iter->second.key.c_str());
        positionV["instrumentKey"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.netPositionS);
        positionV["netPosition_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.netPositionD);
        positionV["netPosition_d"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.netAvgPriceS);
        positionV["netAvgPrice_s"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.netAvgPriceD);
        positionV["netAvgPrice_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountS);
        positionV["floatAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountD);
        positionV["floatAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.underwayNetPositionS);
        positionV["underwayNetPosition"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.underwayAbsPositionS);
        positionV["underwayAbPosition"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.liquidationPrice);
        positionV["liquidationPrice_d"] = web::json::value::string(data);

        if (type == "physical") {
            if (fabs(iter->second.netPositionD) >= MINDOUBLE) {
                if (fabs(iter->second.netPositionS / iter->second.netPositionD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.netPositionS);
                    positionV["netPosition_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.netPositionD);
                    positionV["netPosition_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.netPositionS - iter->second.netPositionD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.netPositionS);
                    positionV["netPosition_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.netPositionD);
                    positionV["netPosition_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.netAvgPriceD) >= MINDOUBLE) {
                if (fabs(iter->second.netAvgPriceS / iter->second.netAvgPriceD - 1) >= 0.05) {
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceS);
                    positionV["netAvgPrice_s"] = web::json::value::string(data);
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceD);
                    positionV["netAvgPrice_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.netAvgPriceS - iter->second.netAvgPriceD) >= MINDOUBLE) {
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceS);
                    positionV["netAvgPrice_s"] = web::json::value::string(data);
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceD);
                    positionV["netAvgPrice_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.floatAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.floatAmountS / iter->second.floatAmountD - 1) > 0.01) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    positionV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    positionV["floatAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    positionV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    positionV["floatAmount_d"] = web::json::value::string(data);
                }
            }
        }
        uFuturePositionV[iter->second.symbol] = positionV;
    }

    if (uFuturePositionV.size() == 0) {
        detailV["UFuturePositionChecked"] = web::json::value::object();
    } else {
        detailV["UFuturePositionChecked"] = uFuturePositionV;
    }


    web::json::value cFuturePositionV;
    for (auto iter = mCFuturePosition.begin(); iter != mCFuturePosition.end(); ++iter) {
        web::json::value positionV;
        char data[24];
        sprintf(data, "%s", iter->second.key.c_str());
        positionV["instrumentKey"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.netPositionS);
        positionV["netPosition_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.netPositionD);
        positionV["netPosition_d"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.netAvgPriceS);
        positionV["netAvgPrice_s"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.netAvgPriceD);
        positionV["netAvgPrice_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountS);
        positionV["floatAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountD);
        positionV["floatAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.underwayNetPositionS);
        positionV["underwayNetPosition"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.underwayAbsPositionS);
        positionV["underwayAbPosition"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.liquidationPrice);
        positionV["liquidationPrice_d"] = web::json::value::string(data);

        if (type == "physical") {
            if (fabs(iter->second.netPositionD) >= MINDOUBLE) {
                if (fabs(iter->second.netPositionS / iter->second.netPositionD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.netPositionS);
                    positionV["netPosition_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.netPositionD);
                    positionV["netPosition_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.netPositionS - iter->second.netPositionD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.netPositionS);
                    positionV["netPosition_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.netPositionD);
                    positionV["netPosition_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.netAvgPriceD) >= MINDOUBLE) {
                if (fabs(iter->second.netAvgPriceS / iter->second.netAvgPriceD - 1) >= 0.05) {
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceS);
                    positionV["netAvgPrice_s"] = web::json::value::string(data);
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceD);
                    positionV["netAvgPrice_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.netAvgPriceS - iter->second.netAvgPriceD) >= MINDOUBLE) {
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceS);
                    positionV["netAvgPrice_s"] = web::json::value::string(data);
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceD);
                    positionV["netAvgPrice_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.floatAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.floatAmountS / iter->second.floatAmountD - 1) > 0.01) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    positionV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    positionV["floatAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    positionV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    positionV["floatAmount_d"] = web::json::value::string(data);
                }
            }
        }
        cFuturePositionV[iter->second.symbol] = positionV;
    }

    if (cFuturePositionV.size() == 0) {
        detailV["CFuturePositionChecked"] = web::json::value::object();
    } else {
        detailV["CFuturePositionChecked"] = cFuturePositionV;
    }

    // delivery position
    web::json::value deliveryPositionV;
    for (auto iter = mDeliveryPosition.begin(); iter != mDeliveryPosition.end(); ++iter) {
        web::json::value positionV;
        char data[24];
        sprintf(data, "%s", iter->second.key.c_str());
        positionV["instrumentKey"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.netPositionS);
        positionV["netPosition_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.netPositionD);
        positionV["netPosition_d"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.netAvgPriceS);
        positionV["netAvgPrice_s"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.netAvgPriceD);
        positionV["netAvgPrice_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountS);
        positionV["floatAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountD);
        positionV["floatAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.underwayNetPositionS);
        positionV["underwayNetPosition"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.underwayAbsPositionS);
        positionV["underwayAbPosition"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.liquidationPrice);
        positionV["liquidationPrice_d"] = web::json::value::string(data);

        if (type == "physical") {
            if (fabs(iter->second.netPositionD) >= MINDOUBLE) {
                if (fabs(iter->second.netPositionS / iter->second.netPositionD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.netPositionS);
                    positionV["netPosition_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.netPositionD);
                    positionV["netPosition_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.netPositionS - iter->second.netPositionD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.netPositionS);
                    positionV["netPosition_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.netPositionD);
                    positionV["netPosition_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.netAvgPriceD) >= MINDOUBLE) {
                if (fabs(iter->second.netAvgPriceS / iter->second.netAvgPriceD - 1) >= 0.05) {
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceS);
                    positionV["netAvgPrice_s"] = web::json::value::string(data);
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceD);
                    positionV["netAvgPrice_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.netAvgPriceS - iter->second.netAvgPriceD) >= MINDOUBLE) {
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceS);
                    positionV["netAvgPrice_s"] = web::json::value::string(data);
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceD);
                    positionV["netAvgPrice_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.floatAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.floatAmountS / iter->second.floatAmountD - 1) > 0.01) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    positionV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    positionV["floatAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    positionV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    positionV["floatAmount_d"] = web::json::value::string(data);
                }
            }
        }
        deliveryPositionV[iter->second.symbol] = positionV;
    }

    if (deliveryPositionV.size() == 0) {
        detailV["DeliveryPositionChecked"] = web::json::value::object();
    } else {
        detailV["DeliveryPositionChecked"] = deliveryPositionV;
    }

    // perpetual position
    web::json::value perpetualPositionV;
    for (auto iter = mPerpetualPosition.begin(); iter != mPerpetualPosition.end(); ++iter) {
        web::json::value positionV;
        char data[24];
        sprintf(data, "%s", iter->second.key.c_str());
        positionV["instrumentKey"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.netPositionS);
        positionV["netPosition_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.netPositionD);
        positionV["netPosition_d"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.netAvgPriceS);
        positionV["netAvgPrice_s"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.netAvgPriceD);
        positionV["netAvgPrice_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountS);
        positionV["floatAmount_s"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.floatAmountD);
        positionV["floatAmount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.underwayNetPositionS);
        positionV["underwayNetPosition"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.underwayAbsPositionS);
        positionV["underwayAbPosition"] = web::json::value::string(data);
        sprintf(data, "%.11f|0", iter->second.liquidationPrice);
        positionV["liquidationPrice_d"] = web::json::value::string(data);

        if (type == "physical") {
            if (fabs(iter->second.netPositionD) >= MINDOUBLE) {
                if (fabs(iter->second.netPositionS / iter->second.netPositionD - 1) >= 0.05) {
                    sprintf(data, "%f|1", iter->second.netPositionS);
                    positionV["netPosition_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.netPositionD);
                    positionV["netPosition_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.netPositionS - iter->second.netPositionD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.netPositionS);
                    positionV["netPosition_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.netPositionD);
                    positionV["netPosition_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.netAvgPriceD) >= MINDOUBLE) {
                if (fabs(iter->second.netAvgPriceS / iter->second.netAvgPriceD - 1) >= 0.05) {
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceS);
                    positionV["netAvgPrice_s"] = web::json::value::string(data);
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceD);
                    positionV["netAvgPrice_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.netAvgPriceS - iter->second.netAvgPriceD) >= MINDOUBLE) {
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceS);
                    positionV["netAvgPrice_s"] = web::json::value::string(data);
                    sprintf(data, "%.11f|1", iter->second.netAvgPriceD);
                    positionV["netAvgPrice_d"] = web::json::value::string(data);
                }
            }

            if (fabs(iter->second.floatAmountD) >= MINDOUBLE) {
                if (fabs(iter->second.floatAmountS / iter->second.floatAmountD - 1) > 0.01) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    positionV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    positionV["floatAmount_d"] = web::json::value::string(data);
                }
            } else {
                if (fabs(iter->second.floatAmountS - iter->second.floatAmountD) >= MINDOUBLE) {
                    sprintf(data, "%f|1", iter->second.floatAmountS);
                    positionV["floatAmount_s"] = web::json::value::string(data);
                    sprintf(data, "%f|1", iter->second.floatAmountD);
                    positionV["floatAmount_d"] = web::json::value::string(data);
                }
            }
        }
        perpetualPositionV[iter->second.symbol] = positionV;
    }

    if (perpetualPositionV.size() == 0) {
        detailV["PerpetualPositionChecked"] = web::json::value::object();
    } else {
        detailV["PerpetualPositionChecked"] = perpetualPositionV;
    }

    web::json::value exposureV;
    for (auto iter = mExposure.begin(); iter != mExposure.end(); ++iter) {
        web::json::value v;
        char data[48];
        sprintf(data, "%f|0", iter->second.initialAmount);
        v["InitialAmount"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.initialValue);
        v["InitialValue"] = web::json::value::string(data);

        if (type == "physical") {
            sprintf(data, "%f|0", iter->second.exposureAmountD);
            v["ExposureAmount"] = web::json::value::string(data);
            if (iter->first == maxRiskExposureAssetD) {
                sprintf(data, "%f|0|MaxExposureValue", iter->second.exposureValueD);
                v["ExposureValue"] = web::json::value::string(data);
            } else {
                sprintf(data, "%f|0", iter->second.exposureValueD);
                v["ExposureValue"] = web::json::value::string(data);
            }
            sprintf(data, "%f|0", iter->second.deltaAmountD);
            v["DeltaAmount"] = web::json::value::string(data);
            sprintf(data, "%f|0", iter->second.deltaValueD);
            v["DeltaValue"] = web::json::value::string(data);
        } else {
            sprintf(data, "%f|0", iter->second.exposureAmountS);
            v["ExposureAmount"] = web::json::value::string(data);
            if (iter->first == maxRiskExposureAssetS) {
                sprintf(data, "%f|0|MaxExposureValue", iter->second.exposureValueS);
                v["ExposureValue"] = web::json::value::string(data);
            } else {
                sprintf(data, "%f|0", iter->second.exposureValueS);
                v["ExposureValue"] = web::json::value::string(data);
            }
            sprintf(data, "%f|0", iter->second.deltaAmountS);
            v["DeltaAmount"] = web::json::value::string(data);
            sprintf(data, "%f|0", iter->second.deltaValueS);
            v["DeltaValue"] = web::json::value::string(data);
        }

        exposureV[iter->first] = v;
    }

    if (exposureV.size() == 0) {
        detailV["ExposureFormChecked"] = web::json::value::object();
    } else {
        detailV["ExposureFormChecked"] = exposureV;
    }

    web::json::value savingAssetV;
    for (auto iter = mSavAsset.begin(); iter != mSavAsset.end(); ++iter) {
        web::json::value v;
        char data[24];
        sprintf(data, "%s|0", iter->second.asset.c_str());
        v["asset"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.amountD);
        v["amount_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.amountInBTCD);
        v["amountInBTC_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.amountInUSDTD);
        v["amountInUSDT_d"] = web::json::value::string(data);
        savingAssetV[iter->first] = v;
    }

    if (savingAssetV.size() == 0) {
        detailV["SavingAssetChecked"] = web::json::value::object();
    } else {
        detailV["SavingAssetChecked"] = savingAssetV;
    }

    web::json::value marginAccountV;
    char dataMargin[24];
    sprintf(dataMargin, "%f|0", totalMarAsset.marginLevel);
    marginAccountV["marginLevel_d"] = web::json::value::string(dataMargin);
    sprintf(dataMargin, "%f|0", totalMarAsset.totalAssetOfBtc);
    marginAccountV["totalAssetOfBtc_d"] = web::json::value::string(dataMargin);
    sprintf(dataMargin, "%f|0", totalMarAsset.totalLiabilityOfBtc);
    marginAccountV["totalLiabilityOfBtc_d"] = web::json::value::string(dataMargin);
    sprintf(dataMargin, "%f|0", totalMarAsset.totalNetAssetOfBtc);
    marginAccountV["totalNetAssetOfBtc_d"] = web::json::value::string(dataMargin);
    
    /*
    if (totalMarAsset.marginLevel <= MINDOUBLE && totalMarAsset.totalAssetOfBtc <= MINDOUBLE && totalMarAsset.totalLiabilityOfBtc <= MINDOUBLE && totalMarAsset.totalNetAssetOfBtc <= MINDOUBLE) {
        detailV["MarginAccountChecked"] = web::json::value::object();
    } else {
        detailV["MarginAccountChecked"] = marginAccountV;
    }
    */

    if (totalMarAsset.marginLevel > MINDOUBLE || totalMarAsset.totalAssetOfBtc > MINDOUBLE || totalMarAsset.totalLiabilityOfBtc > MINDOUBLE || totalMarAsset.totalNetAssetOfBtc > MINDOUBLE) {
        detailV["MarginAccountChecked"] = marginAccountV;
    }

    web::json::value marginAssetV;
    for (auto iter = mMarAsset.begin(); iter != mMarAsset.end(); ++iter) {
        web::json::value v;
        char data[24];
        sprintf(data, "%s|0", iter->second.asset.c_str());
        v["asset"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.borrowed);
        v["borrowed_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.free);
        v["free_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.interest);
        v["interest_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.locked);
        v["locked_d"] = web::json::value::string(data);
        sprintf(data, "%f|0", iter->second.netAsset);
        v["netAsset_d"] = web::json::value::string(data);
        marginAssetV[iter->first] = v;
    }

    /*
    if (marginAssetV.size() == 0) {
        detailV["MarginAssetChecked"] = web::json::value::object();
    } else {
        detailV["MarginAssetChecked"] = marginAssetV;
    }
    */

    if (marginAssetV.size() > 0) {
        detailV["MarginAssetChecked"] = marginAssetV;
    }


    vector<web::json::value> riskV;
    char data[24];
    web::json::value productNameV;
    productNameV["key"] = web::json::value::string("ProductName");
    productNameV["alert"] = web::json::value::boolean(false);
    productNameV["enable"] = web::json::value::boolean(true);
    productNameV["value"] = web::json::value::string(name);
    riskV.push_back(productNameV);

    web::json::value accountIdV;    
    sprintf(data, "%d", customerId);
    accountIdV["key"] = web::json::value::string("AccountID");
    accountIdV["alert"] = web::json::value::boolean(false);
    accountIdV["enable"] = web::json::value::boolean(true);
    accountIdV["value"] = web::json::value::string(data);
    riskV.push_back(accountIdV);

    web::json::value strategyIdV;
    strategyIdV["key"] = web::json::value::string("StrategyID");
    strategyIdV["alert"] = web::json::value::boolean(false);
    vector<web::json::value> vStrategyIdV;
    for (size_t i = 0; i < riskInfo.vStrategyId.size(); ++i) {
        vStrategyIdV.push_back(web::json::value::number(riskInfo.vStrategyId[i]));
    }
    strategyIdV["enable"] = web::json::value::boolean(false);
    strategyIdV["value"] = web::json::value::array(vStrategyIdV);
    riskV.push_back(strategyIdV);

    web::json::value traderIdV;
    traderIdV["key"] = web::json::value::string("TraderID");
    traderIdV["alert"] = web::json::value::boolean(false);
    vector<web::json::value> vTraderIdV;
    for (size_t i = 0; i < riskInfo.vTraderId.size(); ++i) {
        vTraderIdV.push_back(web::json::value::number(riskInfo.vTraderId[i]));
    }
    traderIdV["enable"] = web::json::value::boolean(false);
    traderIdV["value"] = web::json::value::array(vTraderIdV);
    riskV.push_back(traderIdV);

    web::json::value baseAssetV;
    baseAssetV["key"] = web::json::value::string("BaseAsset");
    baseAssetV["alert"] = web::json::value::boolean(false);
    baseAssetV["enable"] = web::json::value::boolean(true);
    baseAssetV["value"] = web::json::value::string(baseAsset);
    riskV.push_back(baseAssetV);

    web::json::value timeV;
    timeV["key"] = web::json::value::string("Time");
    timeV["alert"] = web::json::value::boolean(false);
    timeV["enable"] = web::json::value::boolean(true);
    timeV["value"] = web::json::value::string(CovertToUtcStr(riskInfo.tsLocal));
    riskV.push_back(timeV);

    web::json::value updateTimeDV;
    updateTimeDV["key"] = web::json::value::string("UpdateTime_D");
    updateTimeDV["alert"] = web::json::value::boolean(false);
    updateTimeDV["enable"] = web::json::value::boolean(true);
    updateTimeDV["value"] = web::json::value::string(CovertToUtcStr(riskInfo.dUpdateTime));
    riskV.push_back(updateTimeDV);

    web::json::value updateTimeSV;
    updateTimeSV["key"] = web::json::value::string("UpdateTime_S");
    updateTimeSV["alert"] = web::json::value::boolean(false);
    updateTimeSV["enable"] = web::json::value::boolean(true);
    updateTimeSV["value"] = web::json::value::string(CovertToUtcStr(riskInfo.sUpdateTime));
    riskV.push_back(updateTimeSV);

    web::json::value netValueDV;    
    sprintf(data, "%f", riskInfo.netValueD);
    netValueDV["key"] = web::json::value::string("NetValue_D");
    netValueDV["alert"] = web::json::value::boolean(false);
    if (type == "physical") {
        netValueDV["enable"] = web::json::value::boolean(true);
    } else {
        netValueDV["enable"] = web::json::value::boolean(false);
    } 
    netValueDV["value"] = web::json::value::string(data);
    riskV.push_back(netValueDV);

    web::json::value netValueSV;
    sprintf(data, "%f", riskInfo.netValueS);
    netValueSV["key"] = web::json::value::string("NetValue_S");
    netValueSV["alert"] = web::json::value::boolean(false);
    if (type == "strategy") {
        netValueSV["enable"] = web::json::value::boolean(true);
    } else {
        netValueSV["enable"] = web::json::value::boolean(false);
    }
    netValueSV["value"] = web::json::value::string(data);
    riskV.push_back(netValueSV);

    web::json::value netValueGrowthRateDV;    
    sprintf(data, "%f", riskInfo.netValueGrowthRateD);
    netValueGrowthRateDV["key"] = web::json::value::string("NetValueGrowthRate_D");
    netValueGrowthRateDV["alert"] = web::json::value::boolean(false);
    netValueGrowthRateDV["enable"] = web::json::value::boolean(false);
    netValueGrowthRateDV["value"] = web::json::value::string(data);
    riskV.push_back(netValueGrowthRateDV);

    web::json::value netValueGrowthRateSV;    
    sprintf(data, "%f", riskInfo.netValueGrowthRateS);
    netValueGrowthRateSV["key"] = web::json::value::string("NetValueGrowthRate_S");
    netValueGrowthRateSV["alert"] = web::json::value::boolean(false);
    netValueGrowthRateSV["enable"] = web::json::value::boolean(false);
    netValueGrowthRateSV["value"] = web::json::value::string(data);
    riskV.push_back(netValueGrowthRateSV);

    web::json::value maxValueToLoanRatioDV;    
    sprintf(data, "%f", riskInfo.maxValueToLoanRatioD);
    maxValueToLoanRatioDV["key"] = web::json::value::string("MaxValueToLoanRatio_D");
    maxValueToLoanRatioDV["alert"] = web::json::value::boolean(false);
    maxValueToLoanRatioDV["enable"] = web::json::value::boolean(false);
    maxValueToLoanRatioDV["value"] = web::json::value::string(data);
    riskV.push_back(maxValueToLoanRatioDV);

    web::json::value maxValueToLoanRatioSV;    
    sprintf(data, "%f", riskInfo.maxValueToLoanRatioS);
    maxValueToLoanRatioSV["key"] = web::json::value::string("MaxValueToLoanRatio_S");
    maxValueToLoanRatioSV["alert"] = web::json::value::boolean(false);
    maxValueToLoanRatioSV["enable"] = web::json::value::boolean(false);
    maxValueToLoanRatioSV["value"] = web::json::value::string(data);
    riskV.push_back(maxValueToLoanRatioSV);

    web::json::value maxRealLeverageDV;    
    sprintf(data, "%f", riskInfo.maxRealLeverageD);
    maxRealLeverageDV["key"] = web::json::value::string("MaxRealLeverage_D");
    maxRealLeverageDV["alert"] = web::json::value::boolean(false);
    maxRealLeverageDV["value"] = web::json::value::string(data);
    if (type == "physical") {
        maxRealLeverageDV["enable"] = web::json::value::boolean(true);
        vector<web::json::value> leveragePositionDV;
        leveragePositionDV.push_back(web::json::value::string(maxLeverageTabD));
        leveragePositionDV.push_back(web::json::value::string("MaxRealLeverage"));
        maxRealLeverageDV["position"] = web::json::value::array(leveragePositionDV);
    } else {
        maxRealLeverageDV["enable"] = web::json::value::boolean(false);
    }
    riskV.push_back(maxRealLeverageDV);

    web::json::value maxRealLeverageSV;    
    sprintf(data, "%f", riskInfo.maxRealLeverageS);
    maxRealLeverageSV["key"] = web::json::value::string("MaxRealLeverage_S");
    maxRealLeverageSV["alert"] = web::json::value::boolean(false);
    if (type == "strategy") {
        maxRealLeverageSV["enable"] = web::json::value::boolean(true);
        vector<web::json::value> leveragePositionSV;
        leveragePositionSV.push_back(web::json::value::string(maxLeverageTabS));
        leveragePositionSV.push_back(web::json::value::string("MaxRealLeverage"));
        maxRealLeverageSV["position"] = web::json::value::array(leveragePositionSV);
    } else {
        maxRealLeverageSV["enable"] = web::json::value::boolean(false);
    }
    maxRealLeverageSV["value"] = web::json::value::string(data);
    riskV.push_back(maxRealLeverageSV);

    web::json::value riskExposureDV;    
    sprintf(data, "%f", riskInfo.riskExposureD);
    riskExposureDV["key"] = web::json::value::string("RiskExposure_D");
    riskExposureDV["alert"] = web::json::value::boolean(false);
    riskExposureDV["value"] = web::json::value::string(data);
    if (type == "physical") {
        riskExposureDV["enable"] = web::json::value::boolean(true);
        vector<web::json::value> riskPositionDV;
        riskPositionDV.push_back(web::json::value::string(maxRiskExposureTab));
        riskPositionDV.push_back(web::json::value::string("MaxExposureValue"));
        riskExposureDV["position"] = web::json::value::array(riskPositionDV);
    } else {
        riskExposureDV["enable"] = web::json::value::boolean(false);
    }
    riskV.push_back(riskExposureDV);

    web::json::value riskExposureSV;    
    sprintf(data, "%f", riskInfo.riskExposureS);
    riskExposureSV["key"] = web::json::value::string("RiskExposure_S");
    riskExposureSV["alert"] = web::json::value::boolean(false);
    riskExposureSV["value"] = web::json::value::string(data);
    if (type == "strategy") {
        riskExposureSV["enable"] = web::json::value::boolean(true);
        vector<web::json::value> riskPositionSV;
        riskPositionSV.push_back(web::json::value::string(maxRiskExposureTab));
        riskPositionSV.push_back(web::json::value::string("MaxExposureValue"));
        riskExposureSV["position"] = web::json::value::array(riskPositionSV);
    } else {
        riskExposureSV["enable"] = web::json::value::boolean(false);
    }
    riskV.push_back(riskExposureSV);

    web::json::value underwayOrderValueDV;    
    sprintf(data, "%f", riskInfo.underwayOrderValueD);
    underwayOrderValueDV["key"] = web::json::value::string("UnderwayOrderValue_D");
    underwayOrderValueDV["alert"] = web::json::value::boolean(false);
    if (type == "physical") {
        underwayOrderValueDV["enable"] = web::json::value::boolean(true);
    } else {
        underwayOrderValueDV["enable"] = web::json::value::boolean(false);
    }
    underwayOrderValueDV["value"] = web::json::value::string(data);
    riskV.push_back(underwayOrderValueDV);

    web::json::value underwayOrderValueSV;    
    sprintf(data, "%f", riskInfo.underwayOrderValueS);
    underwayOrderValueSV["key"] = web::json::value::string("UnderwayOrderValue_S");
    underwayOrderValueSV["alert"] = web::json::value::boolean(false);
    if (type == "strategy") {
        underwayOrderValueSV["enable"] = web::json::value::boolean(true);
    } else {
        underwayOrderValueSV["enable"] = web::json::value::boolean(false);
    }
    underwayOrderValueSV["value"] = web::json::value::string(data);
    riskV.push_back(underwayOrderValueSV);

    web::json::value verifyRiskV;    
    sprintf(data, "%f", riskInfo.verifyRiskD);
    verifyRiskV["key"] = web::json::value::string("VerifyRisk");
    verifyRiskV["alert"] = web::json::value::boolean(false);
    verifyRiskV["enable"] = web::json::value::boolean(false);
    verifyRiskV["value"] = web::json::value::string(data);
    riskV.push_back(verifyRiskV);

    web::json::value utilizationV;    
    sprintf(data, "%f", riskInfo.utilizationD);
    utilizationV["key"] = web::json::value::string("Utilization");
    utilizationV["alert"] = web::json::value::boolean(false);
    utilizationV["enable"] = web::json::value::boolean(false);
    utilizationV["value"] = web::json::value::string(data);
    riskV.push_back(utilizationV);

    detailV["MainFormChecked"] = web::json::value::array(riskV);

    return detailV;
}

web::json::value BinanceAccountItem::GetPreview() {
    web::json::value previewV;
    previewV["AccountID"] = web::json::value::number(customerId);
    previewV["AccountName"] = web::json::value::string(name);
    previewV["Time"] = web::json::value::string(CovertToUtcStr(riskInfo.tsLocal));
    previewV["BaseAsset"] = web::json::value::string(baseAsset);

    if (type == "physical") {
        previewV["NowNetValue"] = web::json::value::number(riskInfo.netValueD);
        previewV["NetValueGrowthRate"] = web::json::value::number(riskInfo.netValueGrowthRateD);
        previewV["ValueToLoanRatio"] = web::json::value::number(riskInfo.maxValueToLoanRatioD);
        if (hedge) {
            previewV["Leverage"] = web::json::value::number(riskInfo.maxRealLeverageD);
            previewV["RiskExposure"] = web::json::value::number(riskInfo.riskExposureD);
        } else {
            previewV["Leverage"] = web::json::value::number(0);
            previewV["RiskExposure"] = web::json::value::number(0);
        }

        previewV["UnderwayOrderValue"] = web::json::value::number(riskInfo.underwayOrderValueD);
        previewV["Utilization"] = web::json::value::number(riskInfo.utilizationD);
    } else {
        previewV["NowNetValue"] = web::json::value::number(riskInfo.netValueS);
        previewV["NetValueGrowthRate"] = web::json::value::number(riskInfo.netValueGrowthRateS);
        previewV["ValueToLoanRatio"] = web::json::value::number(riskInfo.maxValueToLoanRatioS);
        previewV["Leverage"] = web::json::value::number(riskInfo.maxRealLeverageS);
        previewV["RiskExposure"] = web::json::value::number(riskInfo.riskExposureS);
        previewV["UnderwayOrderValue"] = web::json::value::number(riskInfo.underwayOrderValueS);
        previewV["Utilization"] = web::json::value::number(riskInfo.utilizationS);
    }

    return previewV;
}

vector<MsgCard> BinanceAccountItem::GetAlarmMsg() {
    vector<MsgCard> v;
    int64_t currentTime = gettickcount();

    if (!adapterQuery) {
        string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
        string errMsg = "查询出错: ";
        for (size_t i = 0; i < adapterQueryErrMsg.size(); ++i) {
            errMsg += adapterQueryErrMsg[i] + "   ";
        }

        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
        MsgCard msgCard;
        msgCard.accountId = customerId;
        msgCard.templateId = 1;
        msgCard.title = "QueryAccount";
        msgCard.object = "账户：" + accountName;
        msgCard.datetime = currentTimeStr;
        msgCard.content = errMsg;
        v.emplace_back(msgCard);

        stringstream ss;
        ss << "accountId:" << customerId << " errMsg:" << errMsg;
        LOG_INFO("QueryAccount: %s", ss.str().c_str()); 
    } else {
        if (alarmInfo.leverageThreshold.warning > 0 && alarmInfo.leverageThreshold.alarm >= alarmInfo.leverageThreshold.warning) {
            bool reachWarning = true;
            bool reachAlarm = true;

            if (riskInfo.maxRealLeverageD < alarmInfo.leverageThreshold.warning) {
                reachWarning = false;
                reachAlarm = false;
            } else if (riskInfo.maxRealLeverageD >= alarmInfo.leverageThreshold.warning && riskInfo.maxRealLeverageD < alarmInfo.leverageThreshold.alarm) {
                reachAlarm = false;
            }
     
            if (reachAlarm) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "Leverage大于等于" <<  alarmInfo.leverageThreshold.alarm << " 当前leverage=" << riskInfo.maxRealLeverageD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                MsgCard msgCard;
                msgCard.accountId = customerId;
                msgCard.templateId = 3;
                msgCard.title = "Leverage";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "Leverage大于等于" <<  alarmInfo.leverageThreshold.warning << " 当前leverage=" << riskInfo.maxRealLeverageD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                MsgCard msgCard;
                msgCard.accountId = customerId;
                msgCard.templateId = 2;
                msgCard.title = "Leverage";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }

        if (alarmInfo.riskExposureThreshold.warning > 0 && alarmInfo.riskExposureThreshold.alarm >= alarmInfo.riskExposureThreshold.warning) {
            bool reachWarning = true;
            bool reachAlarm = true;

            if (fabs(riskInfo.riskExposureD) < alarmInfo.riskExposureThreshold.warning) {
                reachWarning = false;
                reachAlarm = false;
            } else if (fabs(riskInfo.riskExposureD) >= alarmInfo.riskExposureThreshold.warning && fabs(riskInfo.riskExposureD) < alarmInfo.riskExposureThreshold.alarm) {
                reachAlarm = false;
            }

            if (reachAlarm) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.alarm << " 当前riskexposure=" << riskInfo.riskExposureD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                MsgCard msgCard;
                msgCard.accountId = customerId;
                msgCard.templateId = 3;
                msgCard.title = "RiskExposure";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.warning << " 当前riskexposure=" << riskInfo.riskExposureD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                MsgCard msgCard;
                msgCard.accountId = customerId;
                msgCard.templateId = 2;
                msgCard.title = "RiskExposure";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }

        if (alarmInfo.riskExposureThreshold.uwarning > 0 && alarmInfo.riskExposureThreshold.ualarm >= alarmInfo.riskExposureThreshold.uwarning) {
            double exposureU = riskInfo.riskExposureD;
            double price = 0.0;
            if (baseAsset != "USDT" && baseAsset != "USD") {
                if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                    price = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                } else if (exchangeStr == "COINBASE") {
                    price = CoinbaseMdMgr::GetInstance().GetAssetPrice(baseAsset);
                }
            } else {
                price = 1;
            }

            if (price > MINDOUBLE) {
		        exposureU *= price;
                bool reachWarning = true;
                bool reachAlarm = true;

                if (fabs(exposureU) < alarmInfo.riskExposureThreshold.uwarning) {
                    reachWarning = false;
                    reachAlarm = false;
                } else if (fabs(exposureU) >= alarmInfo.riskExposureThreshold.uwarning && fabs(exposureU) < alarmInfo.riskExposureThreshold.ualarm) {
                    reachAlarm = false;
                }

                if (reachAlarm) {
                    stringstream ss;
                    string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                    ss << "RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.ualarm << " 当前riskexposure(换成u)=" << exposureU;

                    string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                    MsgCard msgCard;
                    msgCard.accountId = customerId;
                    msgCard.templateId = 3;
                    msgCard.title = "RiskExposure";
                    msgCard.object = "账户：" + accountName;
                    msgCard.datetime = currentTimeStr;
                    msgCard.content = ss.str();
                    v.emplace_back(msgCard);
                } else if (reachWarning) {
                    stringstream ss;
                    string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                    ss << "RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.uwarning << " 当前riskexposure(换成u)=" << exposureU;

                    string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                    MsgCard msgCard;
                    msgCard.accountId = customerId;
                    msgCard.templateId = 2;
                    msgCard.title = "RiskExposure";
                    msgCard.object = "账户：" + accountName;
                    msgCard.datetime = currentTimeStr;
                    msgCard.content = ss.str();
                    v.emplace_back(msgCard);
                }
		    }
        }

        if (alarmInfo.riskExposureThreshold.totalExposureWarning > 0 && alarmInfo.riskExposureThreshold.totalExposureAlarm >= alarmInfo.riskExposureThreshold.totalExposureWarning) {
            bool reachWarning = true;
            bool reachAlarm = true;

            if (fabs(totalExposure) < alarmInfo.riskExposureThreshold.totalExposureWarning) {
                reachWarning = false;
                reachAlarm = false;
            } else if (fabs(totalExposure) >= alarmInfo.riskExposureThreshold.totalExposureWarning && fabs(totalExposure) < alarmInfo.riskExposureThreshold.totalExposureAlarm) {
                reachAlarm = false;
            }

            if (reachAlarm) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "总的RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.totalExposureAlarm << " 当前总riskexposure=" << totalExposure;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                MsgCard msgCard;
                msgCard.accountId = customerId;
                msgCard.templateId = 3;
                msgCard.title = "RiskExposure";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "总的RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.totalExposureWarning << " 当前总riskexposure=" << totalExposure;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                MsgCard msgCard;
                msgCard.accountId = customerId;
                msgCard.templateId = 2;
                msgCard.title = "RiskExposure";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }

        if (alarmInfo.underwayOrderValueThreshold.warning > 0 && alarmInfo.underwayOrderValueThreshold.alarm >= alarmInfo.underwayOrderValueThreshold.warning) {
            bool reachWarning = true;
            bool reachAlarm = true;

            if (fabs(riskInfo.underwayOrderValueD) < alarmInfo.underwayOrderValueThreshold.warning) {
                reachWarning = false;
                reachAlarm = false;
            } else if (fabs(riskInfo.underwayOrderValueD) >= alarmInfo.underwayOrderValueThreshold.warning && fabs(riskInfo.underwayOrderValueD) < alarmInfo.underwayOrderValueThreshold.alarm) {
                reachAlarm = false;
            }
            
            if (reachAlarm) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "UnderwayOrderValueThreshold大于等于" <<  alarmInfo.underwayOrderValueThreshold.alarm << " 当前underwayordervalue=" << riskInfo.underwayOrderValueD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                MsgCard msgCard;
                msgCard.accountId = customerId;
                msgCard.templateId = 3;
                msgCard.title = "UnderwayOrderValueThreshold";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "UnderwayOrderValueThreshold大于等于" <<  alarmInfo.underwayOrderValueThreshold.warning << " 当前underwayordervalue=" << riskInfo.underwayOrderValueD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                MsgCard msgCard;
                msgCard.accountId = customerId;
                msgCard.templateId = 2;
                msgCard.title = "UnderwayOrderValueThreshold";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }

        if (alarmInfo.netValueThreshold.monthValue > 0 && alarmInfo.netValueThreshold.initValue > 0) {
            double netValueU = riskInfo.netValueD;
            if (baseAsset != "USDT" && baseAsset != "USD") {
                double price = 0.0;
                if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                    price = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                } else if (exchangeStr == "COINBASE") {
                    price = CoinbaseMdMgr::GetInstance().GetAssetPrice(baseAsset);
                }
		        if (price < MINDOUBLE) {
		            return v;
		        }
                netValueU *= price;
            }
            double maxValue = max(alarmInfo.netValueThreshold.monthValue, alarmInfo.netValueThreshold.initValue) / 0.8 * alarmInfo.netValueThreshold.percent;
            if (netValueU <= maxValue) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "netValue小于等于" << maxValue << " 当前netValue(换成u)=" << netValueU;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                MsgCard msgCard;
                msgCard.accountId = customerId;
                msgCard.templateId = 3;
                msgCard.title = "NetValue";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }

        // margin rate
        if ((alarmInfo.marginRateThreshold.initialMarginRateWarning > 0 && alarmInfo.marginRateThreshold.initialMarginRateWarning > alarmInfo.marginRateThreshold.initialMarginRateAlarm) || (alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateWarning > 0 && alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateWarning > alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateAlarm)) {
            if (unified == 1) {
                if (exchangeStr == "BINANCE" && unifyMaintenanceMarginRate > 0) {
                    bool reachWarning = true;
                    bool reachAlarm = true;
                    if (unifyMaintenanceMarginRate >= alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateWarning) {
                        reachWarning = false;
                        reachAlarm = false;
                    } else if (unifyMaintenanceMarginRate > alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateAlarm && unifyMaintenanceMarginRate <= alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateWarning) {
                        reachAlarm = false;
                    }

                    if (reachAlarm) {
                        stringstream ss;
                        string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                        ss << "UnifyMaintenanceMarginRate小于等于" <<  alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateAlarm << " 当前unifyMaintenanceMarginRate=" << unifyMaintenanceMarginRate;

                        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                        MsgCard msgCard;
                        msgCard.accountId = customerId;
                        msgCard.templateId = 3;
                        msgCard.title = "UnifyMaintenanceMarginRate";
                        msgCard.object = "账户：" + accountName;
                        msgCard.datetime = currentTimeStr;
                        msgCard.content = ss.str();
                        v.emplace_back(msgCard);
                    } else if (reachWarning) {
                        stringstream ss;
                        string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                        ss << "UnifyMaintenanceMarginRate小于等于" <<  alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateWarning << " 当前unifyMaintenanceMarginRate=" << unifyMaintenanceMarginRate;

                        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                        MsgCard msgCard;
                        msgCard.accountId = customerId;
                        msgCard.templateId = 2;
                        msgCard.title = "UnifyMaintenanceMarginRate";
                        msgCard.object = "账户：" + accountName;
                        msgCard.datetime = currentTimeStr;
                        msgCard.content = ss.str();
                        v.emplace_back(msgCard);
                    }
                } else if (exchangeStr == "GATEIO" && initialMarginRate > 0) {
                    bool reachWarning = true;
                    bool reachAlarm = true;
                    if (initialMarginRate >= alarmInfo.marginRateThreshold.initialMarginRateWarning) {
                        reachWarning = false;
                        reachAlarm = false;
                    } else if (initialMarginRate > alarmInfo.marginRateThreshold.initialMarginRateAlarm && initialMarginRate <= alarmInfo.marginRateThreshold.initialMarginRateWarning) {
                        reachAlarm = false;
                    }

                    if (reachAlarm) {
                        stringstream ss;
                        string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                        ss << "InitialMarginRate小于等于" <<  alarmInfo.marginRateThreshold.initialMarginRateAlarm << " 当前initialMarginRate=" << initialMarginRate;

                        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                        MsgCard msgCard;
                        msgCard.accountId = customerId;
                        msgCard.templateId = 3;
                        msgCard.title = "InitialMarginRate";
                        msgCard.object = "账户：" + accountName;
                        msgCard.datetime = currentTimeStr;
                        msgCard.content = ss.str();
                        v.emplace_back(msgCard);
                    } else if (reachWarning) {
                        stringstream ss;
                        string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                        ss << "InitialMarginRate小于等于" <<  alarmInfo.marginRateThreshold.initialMarginRateWarning << " 当前initialMarginRate=" << initialMarginRate;

                        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
                        MsgCard msgCard;
                        msgCard.accountId = customerId;
                        msgCard.templateId = 2;
                        msgCard.title = "InitialMarginRate";
                        msgCard.object = "账户：" + accountName;
                        msgCard.datetime = currentTimeStr;
                        msgCard.content = ss.str();
                        v.emplace_back(msgCard);
                    }
                }
            }
        }
    }

    return v;
}

bool BinanceAccountItem::GetAdapterQueryStatus() {
    return adapterQuery;
}

vector<MsgCard> BinanceAccountItem::GetFundingRateAlarmMsg() {
    int64_t currentTime = gettickcount();
    string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
    string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);

    vector<MsgCard> v;
    string content = "";
    if (alarmInfo.fundingRateThreshold.alarm > 0) {
        for (auto iter = mPositionFundingRate.begin(); iter != mPositionFundingRate.end(); ++iter) {
            if ((iter->second.flag && iter->second.value > alarmInfo.fundingRateThreshold.alarm) || (!iter->second.flag && iter->second.value < -alarmInfo.fundingRateThreshold.alarm)) {   
                stringstream ss;
                ss << " Symbol: " << iter->first << " 当前持仓=" << iter->second.netPosition << " FundingRate超过阈值:" << alarmInfo.fundingRateThreshold.alarm << " 当前fundingRate=" << iter->second.value;
                content += ss.str();
            }
        }
    }

    if (content.length() > 0) {
        MsgCard msgCard;
        msgCard.accountId = customerId;
        msgCard.templateId = 2;
        msgCard.title = "FundingRate";
        msgCard.object = "账户：" + accountName;
        msgCard.datetime = currentTimeStr;
        msgCard.content = content;
        v.emplace_back(msgCard);
    }
    return v;
}

MsgCard BinanceAccountItem::GetLiquidationPriceAlarmMsg(igmonitor::Position& position) {
    MsgCard msgCard;
    int64_t currentTime = gettickcount();
    string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(customerId);
    string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);

    if (alarmInfo.liquidationPriceThreshold.warning > 0 && alarmInfo.liquidationPriceThreshold.alarm >= alarmInfo.liquidationPriceThreshold.warning) {        
        bool reachWarning = true;
        bool reachAlarm = true;

        string key = position.key;
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        double liquidationPrice = position.liquidationPrice;
        double netAvgPriceD = position.netAvgPriceD;
        double netPositionD = position.netPositionD;

        if (netPositionD > 0) {
            double warningPrice = liquidationPrice * ( 1 + 1 - alarmInfo.liquidationPriceThreshold.warning);
            double alarmPrice = liquidationPrice * ( 1 + 1 - alarmInfo.liquidationPriceThreshold.alarm);
            if (price > 0 && liquidationPrice > 0) {
                if (price > warningPrice) {
                    reachWarning = false;
                    reachAlarm = false;
                } else if (price <= warningPrice && price > alarmPrice) {
                    reachAlarm = false;
                }
            } else {
                reachWarning = false;
                reachAlarm = false;
            }
        } else if (netPositionD < 0) {
            double warningPrice = liquidationPrice * alarmInfo.liquidationPriceThreshold.warning;
            double alarmPrice = liquidationPrice * alarmInfo.liquidationPriceThreshold.alarm;
            if (price > 0 && liquidationPrice > 0) {
                if (price < warningPrice) {
                    reachWarning = false;
                    reachAlarm = false;
                } else if (price >= warningPrice && price < alarmPrice) {
                    reachAlarm = false;
                }
            } else {
                reachWarning = false;
                reachAlarm = false;
            }
        } else {
            reachWarning = false;
            reachAlarm = false;
        }

        if (reachAlarm) {
            stringstream ss;
            string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
            ss << "持仓InstrumentKey:" << key << " 当前价格已接近强平价! 持仓价格:" << netAvgPriceD << " 持仓手数:" << netPositionD << " 强平价格:" << liquidationPrice << " 当前市场价格:" << price;

            msgCard.accountId = customerId;
            msgCard.templateId = 3;
            msgCard.title = "LiquidationPrice";
            msgCard.object = "账户：" + accountName;
            msgCard.datetime = currentTimeStr;
            msgCard.content = ss.str();
        } else if (reachWarning) {
            stringstream ss;
            string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
            ss << "持仓InstrumentKey:" << key << " 当前价格已接近强平价! 持仓价格:" << netAvgPriceD << " 持仓手数:" << netPositionD << " 强平价格:" << liquidationPrice << " 当前市场价格:" << price;

            msgCard.accountId = customerId;
            msgCard.templateId = 2;
            msgCard.title = "LiquidationPrice";
            msgCard.object = "账户：" + accountName;
            msgCard.datetime = currentTimeStr;
            msgCard.content = ss.str();
        }
    }

    return msgCard;
}

vector<MsgCard> BinanceAccountItem::GetPositionLiquidationPriceAlarmMsg() {
    vector<MsgCard> v;

    for (auto iter = mUFuturePosition.begin(); iter != mUFuturePosition.end(); ++iter) {
        MsgCard msgCard = GetLiquidationPriceAlarmMsg(iter->second);
        if (msgCard.accountId > 0) {
            v.emplace_back(msgCard);
        }
    }

    for (auto iter = mCFuturePosition.begin(); iter != mCFuturePosition.end(); ++iter) {
        MsgCard msgCard = GetLiquidationPriceAlarmMsg(iter->second);
        if (msgCard.accountId > 0) {
            v.emplace_back(msgCard);
        }
    }

    for (auto iter = mDeliveryPosition.begin(); iter != mDeliveryPosition.end(); ++iter) {
        MsgCard msgCard = GetLiquidationPriceAlarmMsg(iter->second);
        if (msgCard.accountId > 0) {
            v.emplace_back(msgCard);
        }
    }

    for (auto iter = mPerpetualPosition.begin(); iter != mPerpetualPosition.end(); ++iter) {
        MsgCard msgCard = GetLiquidationPriceAlarmMsg(iter->second);
        if (msgCard.accountId > 0) {
            v.emplace_back(msgCard);
        }
    }

    return v;
}
