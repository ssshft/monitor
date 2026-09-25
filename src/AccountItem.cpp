#include "AccountItem.h"
#include "MdMgr.h"
#include "BinanceAdapterMgr.h"
#include "GateioAdapterMgr.h"
#include "BybitAdapterMgr.h"
#include "OkxAdapterMgr.h"


AccountItem::AccountItem(int id, string n, string ty, string ex, int he, sm::SecurityManager* s) {
    accountId = id;
    name = n;
    type = ty;
    exchangeStr = ex;
    hedge = he;
    unified = 0;
    baseAsset = MonitorConfig::GetInstance().GetBaseAssetById(accountId);

    riskInfo.accountId = accountId;
    riskInfo.name = name;
    riskInfo.baseAsset = baseAsset;
    MINDOUBLE = 0.0000000001;
    adapterQuery = true;

    alarmInfo = MonitorConfig::GetInstance().GetAlarmInfoById(accountId);

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

    orderAlarmMsg = "";

    smc = s;
}

AccountItem::~AccountItem() {
    Clear();
}

void AccountItem::UpdateByAdapter() {
    if (exchangeStr == "BINANCE") {
        UpdateByBinanceAdapter();
    } else if (exchangeStr == "GATEIO") {
        UpdateByGateioAdapter();
    } else if (exchangeStr == "BYBIT") {
        UpdateByBybitAdapter();
    } else if (exchangeStr == "OKX") {
        UpdateByOkxAdapter();
    }
}

void AccountItem::UpdateByBinanceAdapter() {
    BinanceAdapterItem* item = BinanceAdapterMgr::GetInstance().GetAdapterItem(accountId);
    if (item) {
        riskInfo.dUpdateTime = item->GetUpdateTime();
        adapterQuery = item->GetQueryStatus();
        adapterQueryErrMsg = item->GetQueryErrMsg();
        unified = item->isUnified();

        binance::UnifyAccount& unifyAccount = item->GetUnifyAccount();
        totalMarginBalance = unifyAccount.accountEquity;
        unifyMaintenanceMarginRate = unifyAccount.uniMMR;

        // spot
        std::vector<binance::SpotAsset>& vSpotAsset = item->GetSpotAsset();
        for (size_t i = 0; i < vSpotAsset.size(); ++i) {
            igmonitor::Asset asset;
            asset.asset = vSpotAsset[i].assetType;
            asset.frozenAmountD = vSpotAsset[i].locked;
            asset.totalAmountD = vSpotAsset[i].free + vSpotAsset[i].locked;
            asset.netAmountD = asset.totalAmountD;
            asset.availableAmountD = vSpotAsset[i].free;
            asset.underwayOrderValueD = GetUnderwayOrderValue(asset.asset, asset.frozenAmountD);
            mSpotAsset[asset.asset] = asset;            
        }

        
        std::vector<binance::SpotOpenOrder>& vSpotOpenOrder = item->GetSpotOpenOrder();
        for (size_t i = 0; i < vSpotOpenOrder.size(); ++i) {
            std::string symbol = vSpotOpenOrder[i].symbol;
            std::string key = exchangeStr + "|" + symbol;

            double openQty = vSpotOpenOrder[i].origQty - vSpotOpenOrder[i].executedQty;
            auto iter = mSpotOpenOrder.find(key);
            if (iter != mSpotOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } 
            else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mSpotOpenOrder[key] = openOrder;
            }
       }
       

        // ufuture
        std::vector<binance::UFutureAsset>& vUFutureAsset = item->GetUFutureAsset();
        for (size_t i = 0; i < vUFutureAsset.size(); ++i) {
            std::string ass = vUFutureAsset[i].assetType;
            double positionValue = item->GetUAssetPositionValue(ass);
            double floatAmount = item->GetUFloatAmount(ass);

            igmonitor::Asset asset;
            asset.asset = vUFutureAsset[i].assetType;
            asset.positionValueD = positionValue;
            asset.totalAmountD = vUFutureAsset[i].walletBalance;
            asset.floatAmountD = floatAmount;
            asset.marginAmountD = vUFutureAsset[i].initialMargin;
            asset.frozenMarginAmountD = vUFutureAsset[i].openOrderInitialMargin;
            asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
            asset.availableAmountD = vUFutureAsset[i].availableBalance;
            if (fabs(asset.netAmountD) > MINDOUBLE) {
                asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
            }

            mUFutureAsset[asset.asset] = asset;
        }

        std::vector<binance::UFuturePosition>& vUFuturePosition = item->GetUFuturePosition();
        for (size_t i = 0; i < vUFuturePosition.size(); ++i) {
            std::string symbol = vUFuturePosition[i].symbol;
            std::string positionSide = vUFuturePosition[i].positionSide;
            double longFrozenPosition = 0.0;
            double shortFrozenPosition = 0.0;
            item->GetULongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
       
            igmonitor::Position position;
            position.symbol = vUFuturePosition[i].symbol;
            position.netPositionD = vUFuturePosition[i].positionAmt;
            position.netAvgPriceD = vUFuturePosition[i].entryPrice;
            position.floatAmountD = vUFuturePosition[i].unrealizedProfit;
            position.liquidationPrice = vUFuturePosition[i].liquidationPrice;
            std::string key = exchangeStr + "|" + position.symbol;
            position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
            position.underwayAbsPositioD = fabs(position.underwayNetPositionD);

            mUFuturePosition[position.symbol] = position;
        }

        vector<binance::FutureOpenOrder>& vUFutureOpenOrder = item->GetUOpenOrder();
        for (size_t i = 0; i < vUFutureOpenOrder.size(); ++i) {
            std::string symbol = vUFutureOpenOrder[i].symbol;
            std::string key = exchangeStr + "|" + symbol;
            double openQty = vUFutureOpenOrder[i].origQty - vUFutureOpenOrder[i].executedQty;
            auto iter = mUFutureOpenOrder.find(key);
            if (iter != mUFutureOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } 
            else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mUFutureOpenOrder[key] = openOrder;
            }
        }

        // cfuture
        std::vector<binance::CFutureAsset>& vCFutureAsset = item->GetCFutureAsset();
        for (size_t i = 0; i < vCFutureAsset.size(); ++i) {
            std::string ass = vCFutureAsset[i].assetType;
            double positionValue = item->GetCAssetPositionValue(ass);
            double floatAmount = item->GetCFloatAmount(ass);
  
            igmonitor::Asset asset;
            asset.asset = vCFutureAsset[i].assetType;
            asset.positionValueD = positionValue;
            asset.totalAmountD = vCFutureAsset[i].walletBalance;
            asset.floatAmountD = floatAmount;
            asset.marginAmountD = vCFutureAsset[i].initialMargin;
            asset.frozenMarginAmountD = vCFutureAsset[i].openOrderInitialMargin;
            asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
            asset.availableAmountD = vCFutureAsset[i].availableBalance;
            if (fabs(asset.netAmountD) > MINDOUBLE) {
                asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
            }

            mCFutureAsset[asset.asset] = asset;
        }

        std::vector<binance::CFuturePosition>& vCFuturePosition = item->GetCFuturePosition();
        for (size_t i = 0; i < vCFuturePosition.size(); ++i) {
            std::string symbol = vCFuturePosition[i].symbol;
            std::string positionSide = vCFuturePosition[i].positionSide;
            double liquidationPrice = item->GetCPositionLiquidationPrice(symbol, positionSide);
            double longFrozenPosition = 0.0;
            double shortFrozenPosition = 0.0;
            item->GetCLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
 
            igmonitor::Position position;
            position.symbol = vCFuturePosition[i].symbol;
            position.netPositionD = vCFuturePosition[i].positionAmt;
            position.netAvgPriceD = vCFuturePosition[i].entryPrice;
            position.floatAmountD = vCFuturePosition[i].unrealizedProfit;
            position.liquidationPrice = liquidationPrice;

            std::string key = exchangeStr + "|" + position.symbol;
            position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
            position.underwayAbsPositioD = fabs(position.underwayNetPositionD);

            mCFuturePosition[position.symbol] = position;
        }

        std::vector<binance::FutureOpenOrder>& vCFutureOpenOrder = item->GetCOpenOrder();
        for (size_t i = 0; i < vCFutureOpenOrder.size(); ++i) {
            std::string symbol = vCFutureOpenOrder[i].symbol;

            std::string key = exchangeStr + "|" + symbol;
            double openQty = vCFutureOpenOrder[i].origQty - vCFutureOpenOrder[i].executedQty;
            auto iter = mCFutureOpenOrder.find(key);
            if (iter != mCFutureOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } 
            else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mCFutureOpenOrder[key] = openOrder;
            }
        }

        // unified
        if (unified == 1) {
            std::vector<binance::UnifyAsset>& vUnifyAsset = item->GetUnifyAsset();
            for (size_t i = 0; i < vUnifyAsset.size(); ++i) {
                std::string ass = vUnifyAsset[i].asset;
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
                }
                mUFutureAsset[asset.asset] = asset;
            }

            std::vector<binance::UnifyPosition>& vUmUnifyPosition = item->GetUmUnifyPosition();
            for (size_t i = 0; i < vUmUnifyPosition.size(); ++i) {
                std::string symbol = vUmUnifyPosition[i].symbol;
                double longFrozenPosition = 0.0;
                double shortFrozenPosition = 0.0;
                item->GetUmUnifyLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
                igmonitor::Position position;
                position.symbol = vUmUnifyPosition[i].symbol;
                position.netPositionD = vUmUnifyPosition[i].positionAmt;
                position.netAvgPriceD = vUmUnifyPosition[i].entryPrice;
                position.floatAmountD = vUmUnifyPosition[i].unRealizedProfit;
 
                std::string key = exchangeStr + "|" + position.symbol;                
                position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                position.underwayAbsPositioD = fabs(position.underwayNetPositionD);
                mUFuturePosition[position.symbol] = position;
            }

            std::vector<binance::UnifyPosition>& vCmUnifyPosition = item->GetCmUnifyPosition();
            for (size_t i = 0; i < vCmUnifyPosition.size(); ++i) {
                std::string symbol = vCmUnifyPosition[i].symbol;
                double longFrozenPosition = 0.0;
                double shortFrozenPosition = 0.0;
                item->GetCmUnifyLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);

                igmonitor::Position position;
                position.symbol = vCmUnifyPosition[i].symbol;
                position.netPositionD = vCmUnifyPosition[i].positionAmt;
                position.netAvgPriceD = vCmUnifyPosition[i].entryPrice;
                position.floatAmountD = vCmUnifyPosition[i].unRealizedProfit;

                std::string key = exchangeStr + "|" + position.symbol;   
                position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
                position.underwayAbsPositioD = fabs(position.underwayNetPositionD);
                mCFuturePosition[position.symbol] = position;
            }

            std::vector<binance::UnifyOpenOrder>& vUmUnifyOpenOrder = item->GetUmUnifyOpenOrder();
            for (size_t i = 0; i < vUmUnifyOpenOrder.size(); ++i) {
                std::string symbol = vUmUnifyOpenOrder[i].symbol;

                std::string key = exchangeStr + "|" + symbol;   
                double openQty = vUmUnifyOpenOrder[i].origQty - vUmUnifyOpenOrder[i].executedQty;
                auto iter = mUFutureOpenOrder.find(key);
                if (iter != mUFutureOpenOrder.end()) {
                    iter->second.openQty += fabs(openQty);
                } else {
                    igmonitor::OpenOrder openOrder;
                    openOrder.symbol = symbol;
                    openOrder.openQty = openQty;
                    mUFutureOpenOrder[key] = openOrder;
                }
            }

            std::vector<binance::UnifyOpenOrder>& vCmUnifyOpenOrder = item->GetCmUnifyOpenOrder();
            for (size_t i = 0; i < vCmUnifyOpenOrder.size(); ++i) {
                std::string symbol = vCmUnifyOpenOrder[i].symbol;

                std::string key = exchangeStr + "|" + symbol;   
                double openQty = vCmUnifyOpenOrder[i].origQty - vCmUnifyOpenOrder[i].executedQty;
                auto iter = mCFutureOpenOrder.find(key);
                if (iter != mCFutureOpenOrder.end()) {
                    iter->second.openQty += fabs(openQty);
                } else {
                    igmonitor::OpenOrder openOrder;
                    openOrder.symbol = symbol;
                    openOrder.openQty = openQty;
                    mCFutureOpenOrder[key] = openOrder;
                }
            }
        }
    }
}

void AccountItem::UpdateByGateioAdapter() {
    GateioAdapterItem* item = GateioAdapterMgr::GetInstance().GetAdapterItem(accountId);
    if (item) {
        riskInfo.dUpdateTime = item->GetUpdateTime();
        adapterQuery = item->GetQueryStatus();
        adapterQueryErrMsg = item->GetQueryErrMsg();
        unified = item->isUnified();
    
        // spot
        vector<gateio::SpotAsset>& vSpotAsset = item->GetSpotAsset();
        for (size_t i = 0; i < vSpotAsset.size(); ++i) {
            igmonitor::Asset asset;
            asset.asset = vSpotAsset[i].currency;
            asset.frozenAmountD = vSpotAsset[i].locked;
            asset.totalAmountD = vSpotAsset[i].total;
            asset.netAmountD = asset.totalAmountD;
            asset.availableAmountD = vSpotAsset[i].available;
            asset.underwayOrderValueD = GetUnderwayOrderValue(asset.asset, asset.frozenAmountD);
            mSpotAsset[asset.asset] = asset;
        }

        vector<gateio::FutureAsset>& vPerpetualAsset = item->GetPerpetualAsset();
        for (size_t i = 0; i < vPerpetualAsset.size(); ++i) {
            string ass = vPerpetualAsset[i].currency;
            double positionValue = item->GetPerpetualPositionValue(ass);
            double floatAmount = item->GetPerpetualFloatAmount(ass);
 
            igmonitor::Asset asset;
            asset.asset = vPerpetualAsset[i].currency;
            asset.positionValueD = positionValue;
            asset.totalAmountD = vPerpetualAsset[i].total;
            asset.floatAmountD = floatAmount;
            asset.marginAmountD = vPerpetualAsset[i].positionInitialMargin;
            asset.frozenMarginAmountD = vPerpetualAsset[i].orderMargin;
            asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
            asset.availableAmountD = vPerpetualAsset[i].available;
            if (fabs(asset.netAmountD) > MINDOUBLE) {
                asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
            }

            mPerpetualAsset[asset.asset] = asset;
        }

        vector<gateio::FuturePosition>& vPerpetualPosition = item->GetPerpetualPosition();
        for (size_t i = 0; i < vPerpetualPosition.size(); ++i) {
            string symbol = vPerpetualPosition[i].contract;
            double longFrozenPosition = 0.0;
            double shortFrozenPosition = 0.0;
            item->GetPerpetualLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);

            igmonitor::Position position;
            position.symbol = vPerpetualPosition[i].contract;
            position.netPositionD = vPerpetualPosition[i].size;
            position.netAvgPriceD = vPerpetualPosition[i].entryPrice;
            position.floatAmountD = vPerpetualPosition[i].unrealisedPnl;
            position.liquidationPrice = vPerpetualPosition[i].liqPrice;

            std::string key = exchangeStr + "|" + position.symbol;   
            position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
            position.underwayAbsPositioD = fabs(position.underwayNetPositionD);

            mPerpetualPosition[position.symbol] = position;
        }

        vector<gateio::FutureOrder>& vPerpetualOpenOrder = item->GetPerpetualOpenOrder();
        for (size_t i = 0; i < vPerpetualOpenOrder.size(); ++i) {
            string symbol = vPerpetualOpenOrder[i].contract;

            std::string key = exchangeStr + "|" + symbol; 
            double openQty = vPerpetualOpenOrder[i].left;
            auto iter = mPerpetualOpenOrder.find(key);
            if (iter != mPerpetualOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mPerpetualOpenOrder[key] = openOrder;
            }
        }
    }
}

void AccountItem::UpdateByBybitAdapter() {
    BybitAdapterItem* item = BybitAdapterMgr::GetInstance().GetAdapterItem(accountId);
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
    
            igmonitor::Asset asset;
            asset.asset = vAsset[i].coin;
            asset.positionValueD = positionValue;
            asset.totalAmountD = vAsset[i].walletBalance;
            asset.floatAmountD = floatAmount;
            asset.marginAmountD = vAsset[i].totalPositionIM;
            asset.frozenMarginAmountD = vAsset[i].totalOrderIM;
            asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
            if (fabs(asset.netAmountD) > MINDOUBLE) {
                asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
            }

            mUFutureAsset[asset.asset] = asset;
        }

        vector<bybit::Position>& vPosition = item->GetPosition();
        for (size_t i = 0; i < vPosition.size(); ++i) {
            string symbol = vPosition[i].symbol;
            double longFrozenPosition = 0.0;
            double shortFrozenPosition = 0.0;
            item->GetLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
  
            igmonitor::Position position;
            position.symbol = vPosition[i].symbol;
            position.netPositionD = vPosition[i].size;
            position.netAvgPriceD = vPosition[i].avgPrice;
            position.floatAmountD = vPosition[i].unrealisedPnl;
            position.liquidationPrice = vPosition[i].liqPrice;

            std::string key = exchangeStr + "|" + position.symbol; 
            position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
            position.underwayAbsPositioD = fabs(position.underwayNetPositionD);

            mUFuturePosition[position.symbol] = position;
        }

        vector<bybit::Order>& vOpenOrder = item->GetOpenOrder();
        for (size_t i = 0; i < vOpenOrder.size(); ++i) {
            string symbol = vOpenOrder[i].symbol;

            std::string key = exchangeStr + "|" + symbol; 
            double openQty = vOpenOrder[i].qty - vOpenOrder[i].cumExecQty;
            auto iter = mUFutureOpenOrder.find(key);
            if (iter != mUFutureOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mUFutureOpenOrder[key] = openOrder;
            }
        }

    }
}

void AccountItem::UpdateByOkxAdapter() {
    OkxAdapterItem* item = OkxAdapterMgr::GetInstance().GetAdapterItem(accountId);
    if (item) {
        riskInfo.dUpdateTime = item->GetUpdateTime();
        adapterQuery = item->GetQueryStatus();
        adapterQueryErrMsg = item->GetQueryErrMsg();
        unified = item->isUnified();
   
        // asset
        vector<okx::OkxAsset>& vAsset = item->GetAsset();
        for (size_t i = 0; i < vAsset.size(); ++i) {
            string ass = vAsset[i].ccy;
            double positionValue = item->GetPositionValue(ass);
            double floatAmount = item->GetFloatAmount(ass);

            igmonitor::Asset asset;
            asset.asset = vAsset[i].ccy;
            asset.positionValueD = positionValue;
            asset.totalAmountD = vAsset[i].cashBal;
            asset.floatAmountD = floatAmount;  // 暂不使用计算的浮动盈亏，直接用交易所的浮动盈亏
            // asset.floatAmountD = vAsset[i].upl;
            asset.marginAmountD = vAsset[i].availableEq;
            asset.frozenMarginAmountD = vAsset[i].frozenBal;
            asset.netAmountD = asset.totalAmountD + asset.floatAmountD;
            if (fabs(asset.netAmountD) > MINDOUBLE) {
                asset.realLeverageRatioD = fabs(asset.positionValueD / asset.netAmountD);
            }

            mUFutureAsset[asset.asset] = asset;
        }

        vector<okx::OkxPosition>& vPosition = item->GetPosition();
        for (size_t i = 0; i < vPosition.size(); ++i) {
            string symbol = vPosition[i].instId;
            double longFrozenPosition = 0.0;
            double shortFrozenPosition = 0.0;
            item->GetLongShortFrozenPosition(symbol, longFrozenPosition, shortFrozenPosition);
 
            igmonitor::Position position;
            position.symbol = vPosition[i].instId;
            position.netPositionD = vPosition[i].pos;
            position.netAvgPriceD = vPosition[i].avgPx;
            position.floatAmountD = vPosition[i].upl;
            position.liquidationPrice = vPosition[i].liqPx;

            std::string key = exchangeStr + "|" + position.symbol; 
            position.underwayNetPositionD = fabs(longFrozenPosition) + fabs(shortFrozenPosition);
            position.underwayAbsPositioD = fabs(position.underwayNetPositionD);

            mUFuturePosition[position.symbol] = position;
        }

        vector<okx::OkxOrder>& vOpenOrder = item->GetOpenOrder();
        for (size_t i = 0; i < vOpenOrder.size(); ++i) {
            string symbol = vOpenOrder[i].instId;

            std::string key = exchangeStr + "|" + symbol; 
            double openQty = vOpenOrder[i].sz - vOpenOrder[i].accFillSz;
            auto iter = mUFutureOpenOrder.find(key);
            if (iter != mUFutureOpenOrder.end()) {
                iter->second.openQty += fabs(openQty);
            } else {
                igmonitor::OpenOrder openOrder;
                openOrder.symbol = symbol;
                openOrder.openQty = openQty;
                mUFutureOpenOrder[key] = openOrder;
            }
        }
    }
}

void AccountItem::ClearZero() {
    // clear asset
    for (auto iter = mSpotAsset.begin(); iter != mSpotAsset.end();) {
        if (fabs(iter->second.netAmountD) <= MINDOUBLE) {
            mSpotAsset.erase(iter++);
        } else {
            iter++;
        }
    }

    for (auto iter = mUFutureAsset.begin(); iter != mUFutureAsset.end();) {
        if (fabs(iter->second.netAmountD) <= MINDOUBLE) {
            mUFutureAsset.erase(iter++);
        } else {
            iter++;
        }
    }

    for (auto iter = mCFutureAsset.begin(); iter != mCFutureAsset.end();) {
        if (fabs(iter->second.netAmountD) <= MINDOUBLE) {
            mCFutureAsset.erase(iter++);
        } else {
            iter++;
        }
    }

    // clear position
    for (auto iter = mUFuturePosition.begin(); iter != mUFuturePosition.end();) {
        if (fabs(iter->second.netPositionD) <= MINDOUBLE) {
            mUFuturePosition.erase(iter++);
        } else {
            iter++;
        }
    }

    for (auto iter = mCFuturePosition.begin(); iter != mCFuturePosition.end();) {
        if (fabs(iter->second.netPositionD) <= MINDOUBLE) {
            mCFuturePosition.erase(iter++);
        } else {
            iter++;
        }
    }
}

void AccountItem::Clear() {
    mSpotAsset.clear();
    mUFutureAsset.clear();
    mCFutureAsset.clear();
    mPerpetualAsset.clear();
    mUFuturePosition.clear();
    mCFuturePosition.clear();
    mPerpetualPosition.clear();
    mSpotOpenOrder.clear();
    mUFutureOpenOrder.clear();
    mCFutureOpenOrder.clear();
    mPerpetualOpenOrder.clear();
    mPerpetualAsset.clear();
    mTotalAsset.clear();
    mExposure.clear();
    mPositionFundingRate.clear();
}

void AccountItem::CalculateTotalAsset() {
    for (auto iter = mSpotAsset.begin(); iter != mSpotAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mUFutureAsset.begin(); iter != mUFutureAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mCFutureAsset.begin(); iter != mCFutureAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mPerpetualAsset.begin(); iter != mPerpetualAsset.end(); ++iter) {
        UpdateTotalAsset(iter->second);
    }

    for (auto iter = mTotalAsset.begin(); iter != mTotalAsset.end(); ++iter) {
        if (fabs(iter->second.netAmountD) > MINDOUBLE) {
            iter->second.realLeverageRatioD = fabs(iter->second.positionValueD / iter->second.netAmountD);
        }
    }
}

void AccountItem::UpdateTotalAsset(igmonitor::Asset& asset) {
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
        mTotalAsset[asset.asset] = asset;
    }
}

void AccountItem::CalculateExposure() {
    for (auto iter = mUFuturePosition.begin(); iter != mUFuturePosition.end(); ++iter) {
        std::string originInstId = iter->second.symbol;
        md::InstrumentInfo info;
        if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], USDT_SWAP, originInstId.c_str(), info)) {
        }
        else if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], USDT_FUTURES, originInstId.c_str(), info)) {
        }

        UpdateExposure(info, iter->second);
        if (info.instTypeEnum == USDT_SWAP) {
            //UpdatePositionFundingRate(iter->second);
        }
    }
   
    for (auto iter = mCFuturePosition.begin(); iter != mCFuturePosition.end(); ++iter) {
        std::string originInstId = iter->second.symbol;
        md::InstrumentInfo info;
        if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], C_SWAP, originInstId.c_str(), info)) {
        }
        else if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], C_FUTURES, originInstId.c_str(), info)) {
        }

        UpdateExposure(info, iter->second);
        if (info.instTypeEnum == C_SWAP) {
            //UpdatePositionFundingRate(iter->second);
        }
    }

    for (auto iter = mPerpetualPosition.begin(); iter != mPerpetualPosition.end(); ++iter) {
        std::string originInstId = iter->second.symbol;
        md::InstrumentInfo info;
        if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], USDT_SWAP, originInstId.c_str(), info)) {
        }
        else if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], C_SWAP, originInstId.c_str(), info)) {
        }

        UpdateExposure(info, iter->second);
        //UpdatePositionFundingRate(iter->second);
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
                double price = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                
                if (price > MINDOUBLE) { 
                    iter->second.exposureValueS = iter->second.exposureAmountS / price;
                    iter->second.exposureValueD = iter->second.exposureAmountD / price;
                    iter->second.deltaAmountS = iter->second.exposureAmountS - iter->second.initialAmount;
                    iter->second.deltaAmountD = iter->second.exposureAmountD - iter->second.initialAmount;
                    iter->second.deltaValueS = iter->second.deltaAmountS / price;
                    iter->second.deltaValueD = iter->second.deltaAmountD / price;
                    iter->second.initialValue = iter->second.initialAmount / price;
                }
            } else {
	            std::string instId = asset + "-" + baseAsset;
	            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], SPOT, instId);
                double price = MdMgr::GetInstance().GetMidPrice(key);
                
                if (price > MINDOUBLE) {
                    iter->second.exposureValueS = iter->second.exposureAmountS * price;
                    iter->second.exposureValueD = iter->second.exposureAmountD * price;
                    iter->second.deltaAmountS = iter->second.exposureAmountS - iter->second.initialAmount;
                    iter->second.deltaAmountD = iter->second.exposureAmountD - iter->second.initialAmount;
                    iter->second.deltaValueS = iter->second.deltaAmountS * price;
                    iter->second.deltaValueD = iter->second.deltaAmountD * price;
                    iter->second.initialValue = iter->second.initialAmount * price;
                } else { // 获取对应usdt的价格，再换算成币
                    if (baseAsset == "USDT" || baseAsset == "USD") {
                        double price = MdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);

                        if (price > MINDOUBLE) { 
                            iter->second.exposureValueS = iter->second.exposureAmountS * price;
                            iter->second.exposureValueD = iter->second.exposureAmountD * price;
                            iter->second.deltaAmountS = iter->second.exposureAmountS - iter->second.initialAmount;
                            iter->second.deltaAmountD = iter->second.exposureAmountD - iter->second.initialAmount;
                            iter->second.deltaValueS = iter->second.deltaAmountS * price;
                            iter->second.deltaValueD = iter->second.deltaAmountD * price;
                            iter->second.initialValue = iter->second.initialAmount * price;
                        }
                    } else {
                        double priceU = MdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                        double priceBaseAsset = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);

                        if (priceBaseAsset > MINDOUBLE) {
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

void AccountItem::UpdateExposure(const md::InstrumentInfo& info, const igmonitor::Position& position) {
    std::string key = crypto::get_instrumentInfo_channel_key(info.exchangeTypeEnum, info.instTypeEnum, info.instId);
    double price = MdMgr::GetInstance().GetMidPrice(key);
    string base = info.base;
    string quote = info.quote;

    auto i = mExposure.find(base);
    if (i != mExposure.end()) {
        if (info.instTypeEnum == USDT_SWAP || info.instTypeEnum == USDT_FUTURES) {
            i->second.exposureAmountS += position.netPositionS * info.value;
            i->second.exposureAmountD += position.netPositionD * info.value;
        } 
        else if (info.instTypeEnum == C_SWAP || info.instTypeEnum == C_FUTURES) {
            if (price > MINDOUBLE) {
                i->second.exposureAmountS += position.netPositionS * info.value / price;
                i->second.exposureAmountD += position.netPositionD * info.value / price;
            }
        }
    } else {
        igmonitor::Exposure baseExposure;
        baseExposure.asset = base;
        if (info.instTypeEnum == USDT_SWAP || info.instTypeEnum == USDT_FUTURES) {
            baseExposure.exposureAmountS = position.netPositionS * info.value;
            baseExposure.exposureAmountD = position.netPositionD * info.value;
        } 
        else if (info.instTypeEnum == C_SWAP || info.instTypeEnum == C_FUTURES) {
            if (price > MINDOUBLE) {
                baseExposure.exposureAmountS = position.netPositionS * info.value / price;
                baseExposure.exposureAmountD = position.netPositionD * info.value / price;
            }
        }
        mExposure[base] = baseExposure;
    }

    auto j = mExposure.find(quote);
    if (j != mExposure.end()) {
        if (info.instTypeEnum == USDT_SWAP || info.instTypeEnum == USDT_FUTURES) {
            j->second.exposureAmountS -= position.netPositionS * price * info.value;
            j->second.exposureAmountD -= position.netPositionD * price * info.value;
        } 
        else if (info.instTypeEnum == C_SWAP || info.instTypeEnum == C_FUTURES) {
            j->second.exposureAmountS -= position.netPositionS * info.value;
            j->second.exposureAmountD -= position.netPositionD * info.value;
        }
    } else {
        igmonitor::Exposure quoteExposure;
        quoteExposure.asset = quote;
        if (info.instTypeEnum == USDT_SWAP || info.instTypeEnum == USDT_FUTURES) {
            quoteExposure.exposureAmountS = -position.netPositionS * price * info.value;
            quoteExposure.exposureAmountD = -position.netPositionD * price * info.value;
        } 
        else if (info.instTypeEnum == C_SWAP || info.instTypeEnum == C_FUTURES) {
            quoteExposure.exposureAmountS = -position.netPositionS * info.value;
            quoteExposure.exposureAmountD = -position.netPositionD * info.value;
        }      
        mExposure[quote] = quoteExposure;
    }
}

void AccountItem::UpdateExposure(igmonitor::Asset& asset) {
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
        mExposure[exposure.asset] = exposure;
    }
}

/*
void AccountItem::UpdatePositionFundingRate(igmonitor::Position& position) {
    if (fabs(position.netPositionD) < MINDOUBLE) {
        return;
    }
    
    igmonitor::PositionFundingRate pfr;
    pfr.symbol = position.symbol;
    pfr.netPosition = position.netPositionD;
    pfr.flag = position.netPositionD > 0 ? true : false;
    pfr.value = BinanceMdMgr::GetInstance().GetFundingRate(position.key);
    LOG_INFO("UpdatePositionFundingRate:  accountId: %d  key:%s  flag:%d  value:%f", accountId, position.key.c_str(), pfr.flag, pfr.value);
    mPositionFundingRate[position.key] = pfr;
}
*/

double AccountItem::GetUnderwayOrderValue(string asset, double frozenAmount) {
    double underwayOrderValue = 0.0;
    if (asset == baseAsset) {
        underwayOrderValue = frozenAmount;
    } else {
        if (asset == "USDT" || asset == "USD") {
            double priceBaseAsset = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
            
            if (priceBaseAsset > MINDOUBLE) {
                underwayOrderValue = frozenAmount / priceBaseAsset;
            }
        } else {
	        std::string instId = asset + "-" + baseAsset;
	        std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], SPOT, instId);
            double price = MdMgr::GetInstance().GetMidPrice(key);

            if (price > 0) {
                underwayOrderValue = frozenAmount * price;
            } else {
                if (baseAsset == "USDT" || baseAsset == "USD") {
                    double price = MdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                    underwayOrderValue = frozenAmount * price;
                } else {
                    double priceU = MdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                    double priceBaseAsset = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);

                    if (priceBaseAsset > MINDOUBLE) {
                        underwayOrderValue = frozenAmount * priceU / priceBaseAsset;
                    }
                }
            }
        }
    }
    return underwayOrderValue;
}

void AccountItem::CalculateRiskInfo() {
    riskInfo.tsLocal = GetCurrentTimeUs();
    totalExposure = 0.0;

    double netValueS = 0.0;
    double netValueD = 0.0;
    double yNetValue = 0.0;
    double totalAvailMargin = 0.0;
    double totalPositionValue = 0.0;
    for (auto iter = mTotalAsset.begin(); iter != mTotalAsset.end(); ++iter) {
        string asset = iter->first;
        if (asset == baseAsset) {
            yNetValue += iter->second.initialAmount;
            netValueS += iter->second.netAmountS;
            netValueD += iter->second.netAmountD;
            totalAvailMargin += iter->second.netAmountD;
            totalPositionValue += iter->second.positionValueD;
        } else {
            if (asset == "USDT" || asset == "USD") {
                double price = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                
                if (price > MINDOUBLE) {
                    yNetValue += iter->second.initialAmount / price;
                    netValueS += iter->second.netAmountS / price;
                    netValueD += iter->second.netAmountD / price;
                    if (iter->second.netAmountD > 0) {
                        totalAvailMargin += iter->second.netAmountD / price;
                    }
                    totalPositionValue += iter->second.positionValueD / price;
                }
            } else {
                std::string instId = asset + "-" + baseAsset;
                std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], SPOT, instId);
                double price = MdMgr::GetInstance().GetMidPrice(key);
                
                if (price > MINDOUBLE) {
                    yNetValue += iter->second.initialAmount * price;
                    netValueS += iter->second.netAmountS * price;
                    netValueD += iter->second.netAmountD * price;
                    if (iter->second.netAmountD > 0) {
                        totalAvailMargin += iter->second.netAmountD * price;
                    }
                    totalPositionValue += iter->second.positionValueD / price;
                } else {
                    if (baseAsset == "USDT" || baseAsset == "USD") {
                        double price = MdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                        
                        yNetValue += iter->second.initialAmount * price;
                        netValueS += iter->second.netAmountS * price;
                        netValueD += iter->second.netAmountD * price;
                        if (iter->second.netAmountD > 0) {
                            totalAvailMargin += iter->second.netAmountD * price;
                        }
                        totalPositionValue += iter->second.positionValueD * price;
                    } else {
                        double priceU = MdMgr::GetInstance().GetAssetPrice(asset, exchangeStr);
                        double priceBaseAsset = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);

                        if (priceBaseAsset > MINDOUBLE) {
                            yNetValue += iter->second.initialAmount * priceU / priceBaseAsset;
                            netValueS += iter->second.netAmountS * priceU / priceBaseAsset;
                            netValueD += iter->second.netAmountD * priceU / priceBaseAsset;
                            if (iter->second.netAmountD > 0) {
                                totalAvailMargin += iter->second.netAmountD * priceU / priceBaseAsset;
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
        BybitAdapterItem* item = BybitAdapterMgr::GetInstance().GetAdapterItem(accountId);
        if (item) {
            bybit::TotalAccountInfo& totalAccountInfo = item->GetTotalAccountInfo();
            double accountRate = totalAccountInfo.accountMMRate;
            double rate = totalPositionValue / totalMarginBalance;
            LOG_INFO("BYBIT accountInfo: accountId:{} accountRate:{} rate:{}", accountId, accountRate, rate);
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
        auto it = alarmInfo.riskExposureThreshold.sNoWarningAsset.find(iter->first);
        if (it != alarmInfo.riskExposureThreshold.sNoWarningAsset.end()) {
            continue;
        }

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
        double underwayOrderValue = 0.0;
        double underwayOrderValued = 0.0;

        double price = 0.0;
        std::string originInstId = iter->second.symbol;
        md::InstrumentInfo info;
        if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], USDT_SWAP, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], USDT_SWAP, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);
        }
        else if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], USDT_FUTURES, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], USDT_FUTURES, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);
        }

        if (baseAsset == "USDT") {
            underwayOrderValue = iter->second.underwayAbsPositionS * price *  info.value;
            underwayOrderValued = iter->second.underwayAbsPositioD * price * info.value;
        } else {
            double priceBaseAsset = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
            if (priceBaseAsset > MINDOUBLE) {
                underwayOrderValue = iter->second.underwayAbsPositionS * price * info.value / priceBaseAsset;
                underwayOrderValued = iter->second.underwayAbsPositioD * price * info.value / priceBaseAsset;
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
        double underwayOrderValue = 0.0;
        double underwayOrderValued = 0.0;

        double price = 0.0;
        std::string originInstId = iter->second.symbol;
        md::InstrumentInfo info;
        if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], C_SWAP, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], C_SWAP, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);
        }
        else if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], C_FUTURES, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], C_FUTURES, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);
        }

        if (baseAsset == "USDT") {
            underwayOrderValue = iter->second.underwayAbsPositionS * info.value;
            underwayOrderValued = iter->second.underwayAbsPositioD * info.value;
        } else {
            if (baseAsset == info.margin) {
                if (price > MINDOUBLE) {
                    underwayOrderValue = iter->second.underwayAbsPositionS * info.value / price;
                    underwayOrderValued = iter->second.underwayAbsPositioD * info.value / price;
                }
            } else {
                double priceBaseAsset = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                if (priceBaseAsset > MINDOUBLE) {
                    underwayOrderValue = iter->second.underwayAbsPositionS * info.value / priceBaseAsset;
                    underwayOrderValued = iter->second.underwayAbsPositioD * info.value / priceBaseAsset;
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

        double price = 0.0;
        std::string originInstId = iter->second.symbol;
        md::InstrumentInfo info;
        if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], USDT_SWAP, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], USDT_SWAP, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);
        }
        else if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], USDT_FUTURES, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], USDT_FUTURES, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);
        }

        if (baseAsset == "USDT") {
            underwayOrderValue = iter->second.openQty * price * info.value;
        } else {
            double priceBaseAsset = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
            if (priceBaseAsset > MINDOUBLE) {
                underwayOrderValue = iter->second.openQty * price * info.value / priceBaseAsset;
            }
        }
        if (underwayOrderValue > underwayOrderValueD) {
            underwayOrderValueD = underwayOrderValue;
        }
        LOG_INFO("underwayOrderValue: accountId: {}  underwayOrderValue: {}", accountId, underwayOrderValue); 
    }

    for (auto iter = mCFutureOpenOrder.begin(); iter != mCFutureOpenOrder.end(); ++iter) {
        double underwayOrderValue = 0.0;

        double price = 0.0;
        std::string originInstId = iter->second.symbol;
        md::InstrumentInfo info;
        if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], C_SWAP, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], C_SWAP, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);
        }
        else if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], C_FUTURES, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], C_FUTURES, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);
        }
        
        if (baseAsset == "USDT") {
            underwayOrderValue = iter->second.openQty * info.value;
        } else {
            if (baseAsset == info.margin) {
                if (price > MINDOUBLE) {
                    underwayOrderValue = iter->second.openQty * info.value / price;
                }
            } else {
                double priceBaseAsset = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                if (priceBaseAsset > MINDOUBLE) {
                    underwayOrderValue = iter->second.openQty * info.value / priceBaseAsset;
                }
            }
        }
        if (underwayOrderValue > underwayOrderValueD) {
            underwayOrderValueD = underwayOrderValue;
        }
        LOG_INFO("underwayOrderValue: accountId: {}  underwayOrderValue: {}", accountId, underwayOrderValue); 
    }

    for (auto iter = mPerpetualOpenOrder.begin(); iter != mPerpetualOpenOrder.end(); ++iter) {
        double underwayOrderValue = 0.0;

        double price = 0.0;
        std::string originInstId = iter->second.symbol;
        md::InstrumentInfo info;
        if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], USDT_SWAP, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], USDT_SWAP, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);
        }
        else if (smc->get_instrument_info(ExchangeTypeStr2EnumMap[exchangeStr], C_SWAP, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], C_SWAP, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);

        }

        if (info.instTypeEnum == USDT_SWAP) {
            if (baseAsset == "USDT") {
                underwayOrderValue = iter->second.openQty * price * info.value;
            } else {
                double priceBaseAsset = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                if (priceBaseAsset > MINDOUBLE) {
                    underwayOrderValue = iter->second.openQty * price * info.value / priceBaseAsset;
                }
            }
        } else if (info.instTypeEnum == C_SWAP) {
            if (baseAsset == "USDT") {
                underwayOrderValue = iter->second.openQty * info.value;
            } else {
                if (baseAsset == info.margin) {
                    if (price > MINDOUBLE) {
                        underwayOrderValue = iter->second.openQty * info.value / price;
                    }
                } else {
                    double priceBaseAsset = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                    if (priceBaseAsset > MINDOUBLE) {
                        underwayOrderValue = iter->second.openQty * info.value / priceBaseAsset;
                    }
                }
            }
        }

        if (underwayOrderValue > underwayOrderValueD) {
            underwayOrderValueD = underwayOrderValue;
        }
        LOG_INFO("underwayOrderValue: accountId: {}  underwayOrderValue: {}", accountId, underwayOrderValue); 
    }

    riskInfo.underwayOrderValueD = underwayOrderValueD;


    LOG_INFO("riskInfo: {}", riskInfo.toString());  
}

string AccountItem::GetAccountType() {
    return type;
}

int AccountItem::GetAccountId() {
    return accountId;
}

string AccountItem::GetExchangeStr() {
    return exchangeStr;
}

igmonitor::RiskInfo& AccountItem::GetRiskInfo() {
    return riskInfo;
}

unordered_map<string, igmonitor::Exposure>& AccountItem::GetExposure() {
    return mExposure;
}

unordered_map<string, igmonitor::Asset>& AccountItem::GetSpotAsset() {
    return mSpotAsset;
}

unordered_map<string, igmonitor::Asset>& AccountItem::GetUFutureAsset() {
    return mUFutureAsset;
}

unordered_map<string, igmonitor::Asset>& AccountItem::GetCFutureAsset() {
    return mCFutureAsset;
}

unordered_map<string, igmonitor::Asset>& AccountItem::GetTotalAsset() {
    return mTotalAsset;
}

unordered_map<string, igmonitor::Position>& AccountItem::GetUFuturePosition() {
    return mUFuturePosition;
}

unordered_map<string, igmonitor::Position>& AccountItem::GetCFuturePosition() {
    return mCFuturePosition;
}

unordered_map<string, igmonitor::PositionFundingRate>& AccountItem::GetPositionFundingRate() {
    return mPositionFundingRate;
}

set<string> AccountItem::GetInstrumentList() {
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

rapidjson::Document AccountItem::GetDetail() {
    rapidjson::Document detailV;
    detailV.SetObject();
    auto& allocator = detailV.GetAllocator();
    
    // account_id字段
    detailV.AddMember("account_id", accountId, allocator);

    // TotalAssetChecked
    rapidjson::Value totalAssetChecked(rapidjson::kObjectType);
    for (auto iter = mTotalAsset.begin(); iter != mTotalAsset.end(); ++iter) {
        rapidjson::Value assetV(rapidjson::kObjectType);
        char data[24];
        
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV.AddMember("asset", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.realLeverageRatioD);
        assetV.AddMember("realLeverageRatio", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.positionValueD);
        assetV.AddMember("positionValue", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV.AddMember("transferAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV.AddMember("transferFrozenAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.netAmountD);
        assetV.AddMember("netAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.availableAmountD);
        assetV.AddMember("availableAmount", rapidjson::Value(data, allocator).Move(), allocator);
                
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV.AddMember("totalAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV.AddMember("frozenAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV.AddMember("floatAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV.AddMember("marginAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV.AddMember("frozenMarginAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV.AddMember("initialAmount", rapidjson::Value(data, allocator).Move(), allocator);
                
        totalAssetChecked.AddMember(rapidjson::Value(iter->second.asset.c_str(), allocator).Move(), assetV, allocator);
    }
    detailV.AddMember("TotalAssetChecked", totalAssetChecked, allocator);

    // SpotAssetChecked
    rapidjson::Value spotAssetChecked(rapidjson::kObjectType);
    for (auto iter = mSpotAsset.begin(); iter != mSpotAsset.end(); ++iter) {
        rapidjson::Value assetV(rapidjson::kObjectType);
        char data[24];
        
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV.AddMember("asset", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.realLeverageRatioD);
        assetV.AddMember("realLeverageRatio", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.positionValueD);
        assetV.AddMember("positionValue", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV.AddMember("transferAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV.AddMember("transferFrozenAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.netAmountD);
        assetV.AddMember("netAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.availableAmountD);
        assetV.AddMember("availableAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV.AddMember("floatAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV.AddMember("initialAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV.AddMember("totalAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV.AddMember("frozenAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV.AddMember("marginAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV.AddMember("frozenMarginAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        spotAssetChecked.AddMember(rapidjson::Value(iter->second.asset.c_str(), allocator).Move(), assetV, allocator);
    }
    detailV.AddMember("SpotAssetChecked", spotAssetChecked, allocator);

    // UFutureAssetChecked
    rapidjson::Value uFutureAssetChecked(rapidjson::kObjectType);
    for (auto iter = mUFutureAsset.begin(); iter != mUFutureAsset.end(); ++iter) {
        rapidjson::Value assetV(rapidjson::kObjectType);
        char data[48];
        
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV.AddMember("asset", rapidjson::Value(data, allocator).Move(), allocator);
        
        if (maxLeverageUD == true && maxLeverageAssetD == iter->first) {
            sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioD);
        } else {
            sprintf(data, "%f|0", iter->second.realLeverageRatioD);
        }
        assetV.AddMember("realLeverageRatio", rapidjson::Value(data, allocator).Move(), allocator);
        
       
        sprintf(data, "%f|0", iter->second.positionValueD);
        assetV.AddMember("positionValue", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV.AddMember("transferAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV.AddMember("transferFrozenAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.netAmountD);
        assetV.AddMember("netAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.availableAmountD);
        assetV.AddMember("availableAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV.AddMember("totalAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV.AddMember("frozenAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV.AddMember("floatAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV.AddMember("marginAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV.AddMember("frozenMarginAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV.AddMember("initialAmount", rapidjson::Value(data, allocator).Move(), allocator);
                
        uFutureAssetChecked.AddMember(rapidjson::Value(iter->second.asset.c_str(), allocator).Move(), assetV, allocator);
    }
    detailV.AddMember("UFutureAssetChecked", uFutureAssetChecked, allocator);

    // CFutureAssetChecked
    rapidjson::Value cFutureAssetChecked(rapidjson::kObjectType);
    for (auto iter = mCFutureAsset.begin(); iter != mCFutureAsset.end(); ++iter) {
        rapidjson::Value assetV(rapidjson::kObjectType);
        char data[48];
        
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV.AddMember("asset", rapidjson::Value(data, allocator).Move(), allocator);
        
        if (maxLeverageUD == false && maxLeverageAssetD == iter->first) {
            sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioD);
        } else {
            sprintf(data, "%f|0", iter->second.realLeverageRatioD);
        }
        assetV.AddMember("realLeverageRatio", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.positionValueD);
        assetV.AddMember("positionValue", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV.AddMember("transferAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV.AddMember("transferFrozenAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.netAmountD);
        assetV.AddMember("netAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.availableAmountD);
        assetV.AddMember("availableAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV.AddMember("totalAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV.AddMember("frozenAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV.AddMember("floatAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV.AddMember("marginAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV.AddMember("frozenMarginAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV.AddMember("initialAmount", rapidjson::Value(data, allocator).Move(), allocator);
                
        cFutureAssetChecked.AddMember(rapidjson::Value(iter->second.asset.c_str(), allocator).Move(), assetV, allocator);
    }
    detailV.AddMember("CFutureAssetChecked", cFutureAssetChecked, allocator);


    // PerpetualAssetChecked
    rapidjson::Value perpetualAssetChecked(rapidjson::kObjectType);
    for (auto iter = mPerpetualAsset.begin(); iter != mPerpetualAsset.end(); ++iter) {
        rapidjson::Value assetV(rapidjson::kObjectType);
        char data[48];
        
        sprintf(data, "%s|0", iter->second.asset.c_str());
        assetV.AddMember("asset", rapidjson::Value(data, allocator).Move(), allocator);
        
  
        if (maxLeverageUD == false && maxLeverageAssetD == iter->first) {
            sprintf(data, "%f|0|MaxRealLeverage", iter->second.realLeverageRatioD);
        } else {
            sprintf(data, "%f|0", iter->second.realLeverageRatioD);
        }
        assetV.AddMember("realLeverageRatio", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.positionValueD);
        assetV.AddMember("positionValue", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.transferAmount);
        assetV.AddMember("transferAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.transferFrozenAmount);
        assetV.AddMember("transferFrozenAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.netAmountD);
        assetV.AddMember("netAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.availableAmountD);
        assetV.AddMember("availableAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.totalAmountD);
        assetV.AddMember("totalAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.frozenAmountD);
        assetV.AddMember("frozenAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.floatAmountD);
        assetV.AddMember("floatAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.marginAmountD);
        assetV.AddMember("marginAmount_d", rapidjson::Value(data, allocator).Move(), allocator);

        sprintf(data, "%f|0", iter->second.frozenMarginAmountD);
        assetV.AddMember("frozenMarginAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.initialAmount);
        assetV.AddMember("initialAmount", rapidjson::Value(data, allocator).Move(), allocator);
                
        perpetualAssetChecked.AddMember(rapidjson::Value(iter->second.asset.c_str(), allocator).Move(), assetV, allocator);
    }
    detailV.AddMember("PerpetualAssetChecked", perpetualAssetChecked, allocator);

    // UFuturePositionChecked
    rapidjson::Value uFuturePositionChecked(rapidjson::kObjectType);
    for (auto iter = mUFuturePosition.begin(); iter != mUFuturePosition.end(); ++iter) {
        rapidjson::Value positionV(rapidjson::kObjectType);
        char data[24];
        
        sprintf(data, "%s", iter->second.symbol.c_str());
        positionV.AddMember("instrumentKey", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.netPositionD);
        positionV.AddMember("netPosition_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%.11f|0", iter->second.netAvgPriceD);
        positionV.AddMember("netAvgPrice_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.floatAmountD);
        positionV.AddMember("floatAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.underwayNetPositionD);
        positionV.AddMember("underwayNetPosition", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.underwayAbsPositionD);
        positionV.AddMember("underwayAbPosition", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%.11f|0", iter->second.liquidationPrice);
        positionV.AddMember("liquidationPrice_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        uFuturePositionChecked.AddMember(rapidjson::Value(iter->second.symbol.c_str(), allocator).Move(), positionV, allocator);
    }
    detailV.AddMember("UFuturePositionChecked", uFuturePositionChecked, allocator);

    // CFuturePositionChecked
    rapidjson::Value cFuturePositionChecked(rapidjson::kObjectType);
    for (auto iter = mCFuturePosition.begin(); iter != mCFuturePosition.end(); ++iter) {
        rapidjson::Value positionV(rapidjson::kObjectType);
        char data[24];
        
        sprintf(data, "%s", iter->second.symbol.c_str());
        positionV.AddMember("instrumentKey", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.netPositionD);
        positionV.AddMember("netPosition_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%.11f|0", iter->second.netAvgPriceD);
        positionV.AddMember("netAvgPrice_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.floatAmountD);
        positionV.AddMember("floatAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.underwayNetPositionD);
        positionV.AddMember("underwayNetPosition", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.underwayAbsPositionD);
        positionV.AddMember("underwayAbPosition", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%.11f|0", iter->second.liquidationPrice);
        positionV.AddMember("liquidationPrice_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        cFuturePositionChecked.AddMember(rapidjson::Value(iter->second.symbol.c_str(), allocator).Move(), positionV, allocator);
    }
    detailV.AddMember("CFuturePositionChecked", cFuturePositionChecked, allocator);


    // PerpetualPositionChecked
    rapidjson::Value perpetualPositionChecked(rapidjson::kObjectType);
    for (auto iter = mPerpetualPosition.begin(); iter != mPerpetualPosition.end(); ++iter) {
        rapidjson::Value positionV(rapidjson::kObjectType);
        char data[24];
        
        sprintf(data, "%s", iter->second.symbol.c_str());
        positionV.AddMember("instrumentKey", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.netPositionD);
        positionV.AddMember("netPosition_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%.11f|0", iter->second.netAvgPriceD);
        positionV.AddMember("netAvgPrice_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.floatAmountD);
        positionV.AddMember("floatAmount_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.underwayNetPositionD);
        positionV.AddMember("underwayNetPosition", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.underwayAbsPositionD);
        positionV.AddMember("underwayAbPosition", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%.11f|0", iter->second.liquidationPrice);
        positionV.AddMember("liquidationPrice_d", rapidjson::Value(data, allocator).Move(), allocator);
        
        perpetualPositionChecked.AddMember(rapidjson::Value(iter->second.symbol.c_str(), allocator).Move(), positionV, allocator);
    }
    detailV.AddMember("PerpetualPositionChecked", perpetualPositionChecked, allocator);

    // ExposureFormChecked
    rapidjson::Value exposureFormChecked(rapidjson::kObjectType);
    for (auto iter = mExposure.begin(); iter != mExposure.end(); ++iter) {
        rapidjson::Value exposureV(rapidjson::kObjectType);
        char data[48];
        
        sprintf(data, "%f|0", iter->second.initialAmount);
        exposureV.AddMember("InitialAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.initialValue);
        exposureV.AddMember("InitialValue", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.exposureAmountD);
        exposureV.AddMember("ExposureAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        if (iter->first == maxRiskExposureAssetD) {
            sprintf(data, "%f|0|MaxExposureValue", iter->second.exposureValueD);
        } else {
            sprintf(data, "%f|0", iter->second.exposureValueD);
        }
        exposureV.AddMember("ExposureValue", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.deltaAmountD);
        exposureV.AddMember("DeltaAmount", rapidjson::Value(data, allocator).Move(), allocator);
        
        sprintf(data, "%f|0", iter->second.deltaValueD);
        exposureV.AddMember("DeltaValue", rapidjson::Value(data, allocator).Move(), allocator);

        exposureFormChecked.AddMember(rapidjson::Value(iter->first.c_str(), allocator).Move(), exposureV, allocator);
    }
    detailV.AddMember("ExposureFormChecked", exposureFormChecked, allocator);

    // MainFormChecked - array
    rapidjson::Value mainFormChecked(rapidjson::kArrayType);
    char data[24];
    
    // ProductName
    rapidjson::Value productNameV(rapidjson::kObjectType);
    productNameV.AddMember("key", "ProductName", allocator);
    productNameV.AddMember("alert", false, allocator);
    productNameV.AddMember("enable", true, allocator);
    productNameV.AddMember("value", rapidjson::Value(name.c_str(), allocator).Move(), allocator);
    mainFormChecked.PushBack(productNameV, allocator);
    
    // AccountID
    rapidjson::Value accountIdV(rapidjson::kObjectType);
    sprintf(data, "%d", accountId);
    accountIdV.AddMember("key", "AccountID", allocator);
    accountIdV.AddMember("alert", false, allocator);
    accountIdV.AddMember("enable", true, allocator);
    accountIdV.AddMember("value", rapidjson::Value(data, allocator).Move(), allocator);
    mainFormChecked.PushBack(accountIdV, allocator);
    
    // BaseAsset
    rapidjson::Value baseAssetV(rapidjson::kObjectType);
    baseAssetV.AddMember("key", "BaseAsset", allocator);
    baseAssetV.AddMember("alert", false, allocator);
    baseAssetV.AddMember("enable", true, allocator);
    baseAssetV.AddMember("value", rapidjson::Value(baseAsset.c_str(), allocator).Move(), allocator);
    mainFormChecked.PushBack(baseAssetV, allocator);
    
    // Time
    rapidjson::Value timeV(rapidjson::kObjectType);
    timeV.AddMember("key", "Time", allocator);
    timeV.AddMember("alert", false, allocator);
    timeV.AddMember("enable", true, allocator);
    timeV.AddMember("value", rapidjson::Value(CovertToUtcStr(riskInfo.tsLocal).c_str(), allocator).Move(), allocator);
    mainFormChecked.PushBack(timeV, allocator);
    
    // UpdateTime_D
    rapidjson::Value updateTimeDV(rapidjson::kObjectType);
    updateTimeDV.AddMember("key", "UpdateTime_D", allocator);
    updateTimeDV.AddMember("alert", false, allocator);
    updateTimeDV.AddMember("enable", true, allocator);
    updateTimeDV.AddMember("value", rapidjson::Value(CovertToUtcStr(riskInfo.dUpdateTime).c_str(), allocator).Move(), allocator);
    mainFormChecked.PushBack(updateTimeDV, allocator);
        
    // NetValue_D
    rapidjson::Value netValueDV(rapidjson::kObjectType);
    sprintf(data, "%f", riskInfo.netValueD);
    netValueDV.AddMember("key", "NetValue_D", allocator);
    netValueDV.AddMember("alert", false, allocator);
    netValueDV.AddMember("enable", type == "physical", allocator);
    netValueDV.AddMember("value", rapidjson::Value(data, allocator).Move(), allocator);
    mainFormChecked.PushBack(netValueDV, allocator);
        
    // NetValueGrowthRate_D
    rapidjson::Value netValueGrowthRateDV(rapidjson::kObjectType);
    sprintf(data, "%f", riskInfo.netValueGrowthRateD);
    netValueGrowthRateDV.AddMember("key", "NetValueGrowthRate_D", allocator);
    netValueGrowthRateDV.AddMember("alert", false, allocator);
    netValueGrowthRateDV.AddMember("enable", false, allocator);
    netValueGrowthRateDV.AddMember("value", rapidjson::Value(data, allocator).Move(), allocator);
    mainFormChecked.PushBack(netValueGrowthRateDV, allocator);
        
    // MaxValueToLoanRatio_D
    rapidjson::Value maxValueToLoanRatioDV(rapidjson::kObjectType);
    sprintf(data, "%f", riskInfo.maxValueToLoanRatioD);
    maxValueToLoanRatioDV.AddMember("key", "MaxValueToLoanRatio_D", allocator);
    maxValueToLoanRatioDV.AddMember("alert", false, allocator);
    maxValueToLoanRatioDV.AddMember("enable", false, allocator);
    maxValueToLoanRatioDV.AddMember("value", rapidjson::Value(data, allocator).Move(), allocator);
    mainFormChecked.PushBack(maxValueToLoanRatioDV, allocator);
        
    // MaxRealLeverage_D
    rapidjson::Value maxRealLeverageDV(rapidjson::kObjectType);
    sprintf(data, "%f", riskInfo.maxRealLeverageD);
    maxRealLeverageDV.AddMember("key", "MaxRealLeverage_D", allocator);
    maxRealLeverageDV.AddMember("alert", false, allocator);
    maxRealLeverageDV.AddMember("enable", type == "physical", allocator);
    
    rapidjson::Value leveragePositionDV(rapidjson::kArrayType);
    leveragePositionDV.PushBack(rapidjson::Value(maxLeverageTabD.c_str(), allocator).Move(), allocator);
    leveragePositionDV.PushBack("MaxRealLeverage", allocator);
    maxRealLeverageDV.AddMember("position", leveragePositionDV, allocator);
    
    maxRealLeverageDV.AddMember("value", rapidjson::Value(data, allocator).Move(), allocator);
    mainFormChecked.PushBack(maxRealLeverageDV, allocator);
        
    // RiskExposure_D
    rapidjson::Value riskExposureDV(rapidjson::kObjectType);
    sprintf(data, "%f", riskInfo.riskExposureD);
    riskExposureDV.AddMember("key", "RiskExposure_D", allocator);
    riskExposureDV.AddMember("alert", false, allocator);
    riskExposureDV.AddMember("enable", type == "physical", allocator);
    
    rapidjson::Value riskPositionDV(rapidjson::kArrayType);
    riskPositionDV.PushBack(rapidjson::Value(maxRiskExposureTab.c_str(), allocator).Move(), allocator);
    riskPositionDV.PushBack("MaxExposureValue", allocator);
    riskExposureDV.AddMember("position", riskPositionDV, allocator);
    
    riskExposureDV.AddMember("value", rapidjson::Value(data, allocator).Move(), allocator);
    mainFormChecked.PushBack(riskExposureDV, allocator);
        
    // UnderwayOrderValue_D
    rapidjson::Value underwayOrderValueDV(rapidjson::kObjectType);
    sprintf(data, "%f", riskInfo.underwayOrderValueD);
    underwayOrderValueDV.AddMember("key", "UnderwayOrderValue_D", allocator);
    underwayOrderValueDV.AddMember("alert", false, allocator);
    underwayOrderValueDV.AddMember("enable", type == "physical", allocator);
    underwayOrderValueDV.AddMember("value", rapidjson::Value(data, allocator).Move(), allocator);
    mainFormChecked.PushBack(underwayOrderValueDV, allocator);
        
    // VerifyRisk
    rapidjson::Value verifyRiskV(rapidjson::kObjectType);
    sprintf(data, "%f", riskInfo.verifyRiskD);
    verifyRiskV.AddMember("key", "VerifyRisk", allocator);
    verifyRiskV.AddMember("alert", false, allocator);
    verifyRiskV.AddMember("enable", false, allocator);
    verifyRiskV.AddMember("value", rapidjson::Value(data, allocator).Move(), allocator);
    mainFormChecked.PushBack(verifyRiskV, allocator);
    
    // Utilization
    rapidjson::Value utilizationV(rapidjson::kObjectType);
    sprintf(data, "%f", riskInfo.utilizationD);
    utilizationV.AddMember("key", "Utilization", allocator);
    utilizationV.AddMember("alert", false, allocator);
    utilizationV.AddMember("enable", false, allocator);
    utilizationV.AddMember("value", rapidjson::Value(data, allocator).Move(), allocator);
    mainFormChecked.PushBack(utilizationV, allocator);
    
    detailV.AddMember("MainFormChecked", mainFormChecked, allocator);
    
    return detailV;
}

rapidjson::Document AccountItem::GetPreview() {
    rapidjson::Document previewV;
    previewV.SetObject();
    auto& allocator = previewV.GetAllocator();
    
    previewV.AddMember("AccountID", accountId, allocator);
    previewV.AddMember("AccountName", rapidjson::Value(name.c_str(), allocator).Move(), allocator);
    previewV.AddMember("Time", rapidjson::Value(CovertToUtcStr(riskInfo.tsLocal).c_str(), allocator).Move(), allocator);
    previewV.AddMember("BaseAsset", rapidjson::Value(baseAsset.c_str(), allocator).Move(), allocator);

    previewV.AddMember("NowNetValue", riskInfo.netValueD, allocator);
    previewV.AddMember("NetValueGrowthRate", riskInfo.netValueGrowthRateD, allocator);
    previewV.AddMember("ValueToLoanRatio", riskInfo.maxValueToLoanRatioD, allocator);
    if (hedge) {
        previewV.AddMember("Leverage", riskInfo.maxRealLeverageD, allocator);
        previewV.AddMember("RiskExposure", riskInfo.riskExposureD, allocator);
    } else {
        previewV.AddMember("Leverage", 0, allocator);
        previewV.AddMember("RiskExposure", 0, allocator);
    }

    previewV.AddMember("UnderwayOrderValue", riskInfo.underwayOrderValueD, allocator);
    previewV.AddMember("Utilization", riskInfo.utilizationD, allocator);

    return previewV;
}

vector<MsgCard> AccountItem::GetAlarmMsg() {
    vector<MsgCard> v;
    int64_t currentTime = crypto::getCurrentTime();

    if (!adapterQuery) {
        string currentTimeStr = CovertToUtcStr(currentTime, false);
        string errMsg = "查询出错: ";
        for (size_t i = 0; i < adapterQueryErrMsg.size(); ++i) {
            errMsg += adapterQueryErrMsg[i] + "   ";
        }

        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
        MsgCard msgCard;
        msgCard.accountId = accountId;
        msgCard.templateId = 1;
        msgCard.title = "QueryAccount";
        msgCard.object = "账户：" + accountName;
        msgCard.datetime = currentTimeStr;
        msgCard.content = errMsg;
        v.emplace_back(msgCard);

        LOG_INFO("QueryAccount accountId: {} errMsg:{}", accountId, errMsg); 
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
                string currentTimeStr = CovertToUtcStr(currentTime , false);
                ss << "Leverage大于等于" <<  alarmInfo.leverageThreshold.alarm << " 当前leverage=" << riskInfo.maxRealLeverageD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                MsgCard msgCard;
                msgCard.accountId = accountId;
                msgCard.templateId = 3;
                msgCard.title = "Leverage";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime, false);
                ss << "Leverage大于等于" <<  alarmInfo.leverageThreshold.warning << " 当前leverage=" << riskInfo.maxRealLeverageD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                MsgCard msgCard;
                msgCard.accountId = accountId;
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
                string currentTimeStr = CovertToUtcStr(currentTime, false);
                ss << "RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.alarm << " 当前riskexposure=" << riskInfo.riskExposureD << " symbol:" << maxRiskExposureAssetD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                MsgCard msgCard;
                msgCard.accountId = accountId;
                msgCard.templateId = 3;
                msgCard.title = "RiskExposure";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime, false);
                ss << "RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.warning << " 当前riskexposure=" << riskInfo.riskExposureD << " symbol:" << maxRiskExposureAssetD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                MsgCard msgCard;
                msgCard.accountId = accountId;
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
                price = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr); 
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
                    string currentTimeStr = CovertToUtcStr(currentTime, false);
                    ss << "RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.ualarm << " 当前riskexposure(换成u)=" << exposureU << " symbol:" << maxRiskExposureAssetD;

                    string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                    MsgCard msgCard;
                    msgCard.accountId = accountId;
                    msgCard.templateId = 3;
                    msgCard.title = "RiskExposure";
                    msgCard.object = "账户：" + accountName;
                    msgCard.datetime = currentTimeStr;
                    msgCard.content = ss.str();
                    v.emplace_back(msgCard);
                } else if (reachWarning) {
                    stringstream ss;
                    string currentTimeStr = CovertToUtcStr(currentTime, false);
                    ss << "RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.uwarning << " 当前riskexposure(换成u)=" << exposureU << " symbol:" << maxRiskExposureAssetD;

                    string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                    MsgCard msgCard;
                    msgCard.accountId = accountId;
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
                string currentTimeStr = CovertToUtcStr(currentTime, false);
                ss << "总的RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.totalExposureAlarm << " 当前总riskexposure=" << totalExposure;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                MsgCard msgCard;
                msgCard.accountId = accountId;
                msgCard.templateId = 3;
                msgCard.title = "RiskExposure";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime, false);
                ss << "总的RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.totalExposureWarning << " 当前总riskexposure=" << totalExposure;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                MsgCard msgCard;
                msgCard.accountId = accountId;
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
                string currentTimeStr = CovertToUtcStr(currentTime, false);
                ss << "UnderwayOrderValueThreshold大于等于" <<  alarmInfo.underwayOrderValueThreshold.alarm << " 当前underwayordervalue=" << riskInfo.underwayOrderValueD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                MsgCard msgCard;
                msgCard.accountId = accountId;
                msgCard.templateId = 3;
                msgCard.title = "UnderwayOrderValueThreshold";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime, false);
                ss << "UnderwayOrderValueThreshold大于等于" <<  alarmInfo.underwayOrderValueThreshold.warning << " 当前underwayordervalue=" << riskInfo.underwayOrderValueD;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                MsgCard msgCard;
                msgCard.accountId = accountId;
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
                double price = MdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                netValueU *= price;
            }
            double maxValue = max(alarmInfo.netValueThreshold.monthValue, alarmInfo.netValueThreshold.initValue) / 0.8 * alarmInfo.netValueThreshold.percent;
            if (netValueU <= maxValue) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "netValue小于等于" << maxValue << " 当前netValue(换成u)=" << netValueU;

                string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                MsgCard msgCard;
                msgCard.accountId = accountId;
                msgCard.templateId = 3;
                msgCard.title = "NetValue";
                msgCard.object = "账户：" + accountName;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }

        // margin rate
        if (alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateWarning > 0 && alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateWarning > alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateAlarm) {
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
                        string currentTimeStr = CovertToUtcStr(currentTime, false);
                        ss << "UnifyMaintenanceMarginRate小于等于" <<  alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateAlarm << " 当前unifyMaintenanceMarginRate=" << unifyMaintenanceMarginRate;

                        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                        MsgCard msgCard;
                        msgCard.accountId = accountId;
                        msgCard.templateId = 3;
                        msgCard.title = "UnifyMaintenanceMarginRate";
                        msgCard.object = "账户：" + accountName;
                        msgCard.datetime = currentTimeStr;
                        msgCard.content = ss.str();
                        v.emplace_back(msgCard);
                    } else if (reachWarning) {
                        stringstream ss;
                        string currentTimeStr = CovertToUtcStr(currentTime, false);
                        ss << "UnifyMaintenanceMarginRate小于等于" <<  alarmInfo.marginRateThreshold.unifyMaintenanceMarginRateWarning << " 当前unifyMaintenanceMarginRate=" << unifyMaintenanceMarginRate;

                        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
                        MsgCard msgCard;
                        msgCard.accountId = accountId;
                        msgCard.templateId = 2;
                        msgCard.title = "UnifyMaintenanceMarginRate";
                        msgCard.object = "账户：" + accountName;
                        msgCard.datetime = currentTimeStr;
                        msgCard.content = ss.str();
                        v.emplace_back(msgCard);
                    }
                } else if (exchangeStr == "GATEIO" && initialMarginRate > 0) {

                }
            }
        }
    }

    return v;
}

bool AccountItem::GetAdapterQueryStatus() {
    return adapterQuery;
}

vector<MsgCard> AccountItem::GetFundingRateAlarmMsg() {
    int64_t currentTime = crypto::getCurrentTime();
    string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
    string currentTimeStr = CovertToUtcStr(currentTime, false);

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
        msgCard.accountId = accountId;
        msgCard.templateId = 2;
        msgCard.title = "FundingRate";
        msgCard.object = "账户：" + accountName;
        msgCard.datetime = currentTimeStr;
        msgCard.content = content;
        v.emplace_back(msgCard);
    }
    return v;
}

MsgCard AccountItem::GetLiquidationPriceAlarmMsg(igmonitor::Position& position) {
    MsgCard msgCard;
    int64_t currentTime = crypto::getCurrentTime();
    string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
    string currentTimeStr = CovertToUtcStr(currentTime , false);

    if (alarmInfo.liquidationPriceThreshold.warning > 0 && alarmInfo.liquidationPriceThreshold.alarm >= alarmInfo.liquidationPriceThreshold.warning) {        
        bool reachWarning = true;
        bool reachAlarm = true;

        string key = position.symbol;
        double price = MdMgr::GetInstance().GetMidPrice(key);
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
            string currentTimeStr = CovertToUtcStr(currentTime, false);
            ss << "持仓InstrumentKey:" << key << " 当前价格已接近强平价! 持仓价格:" << netAvgPriceD << " 持仓手数:" << netPositionD << " 强平价格:" << liquidationPrice << " 当前市场价格:" << price;

            msgCard.accountId = accountId;
            msgCard.templateId = 3;
            msgCard.title = "LiquidationPrice";
            msgCard.object = "账户：" + accountName;
            msgCard.datetime = currentTimeStr;
            msgCard.content = ss.str();
        } else if (reachWarning) {
            stringstream ss;
            string currentTimeStr = CovertToUtcStr(currentTime, false);
            ss << "持仓InstrumentKey:" << key << " 当前价格已接近强平价! 持仓价格:" << netAvgPriceD << " 持仓手数:" << netPositionD << " 强平价格:" << liquidationPrice << " 当前市场价格:" << price;

            msgCard.accountId = accountId;
            msgCard.templateId = 2;
            msgCard.title = "LiquidationPrice";
            msgCard.object = "账户：" + accountName;
            msgCard.datetime = currentTimeStr;
            msgCard.content = ss.str();
        }
    }

    return msgCard;
}

vector<MsgCard> AccountItem::GetPositionLiquidationPriceAlarmMsg() {
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

    for (auto iter = mPerpetualPosition.begin(); iter != mPerpetualPosition.end(); ++iter) {
        MsgCard msgCard = GetLiquidationPriceAlarmMsg(iter->second);
        if (msgCard.accountId > 0) {
            v.emplace_back(msgCard);
        }
    }

    return v;
}
