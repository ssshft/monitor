#include "GateioAdapterItem.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"


GateioAdapterItem::GateioAdapterItem(AccountInfo info) {
    accountInfo = info;
    gateioSpot = new GateioSpot(accountInfo);
    gateioDelivery = new GateioDelivery(accountInfo);
    gateioPerpetual = new GateioPerpetual(accountInfo);
    gateioCrossMargin = new GateioCrossMargin(accountInfo);
    baseAsset = accountInfo.baseAsset;
    updateTime = 0;
    spotEnable = false;
    deliveryEnable = false;
    perpetualEnable = false;
    crossMarginEnable = false;
    string apiPermission = accountInfo.apiPermission;
    vector<string> v;
    SplitString(apiPermission, ",", v);
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == "all") {
            spotEnable = true;
            deliveryEnable = true;
            perpetualEnable = true;
            crossMarginEnable = true;
        } else if (v[i] == "spot") {
            spotEnable = true;
        } else if (v[i] == "delivery") {
            deliveryEnable = true;
        } else if (v[i] == "perpetual") {
            perpetualEnable = true;
        } else if (v[i] == "cross_margin") {
            crossMarginEnable = true;
        }
    }
}

GateioAdapterItem::~GateioAdapterItem() {
    if (gateioSpot) {
        delete gateioSpot;
        gateioSpot = nullptr;
    }
    if (gateioDelivery) {
        delete gateioDelivery;
        gateioDelivery = nullptr;
    }
    if (gateioPerpetual) {
        delete gateioPerpetual;
        gateioPerpetual = nullptr;
    }
    if (gateioCrossMargin) {
        delete gateioCrossMargin;
        gateioCrossMargin = nullptr;
    }

    vSpotAsset.clear();
    vDeliveryAsset.clear();
    vDeliveryPosition.clear();
    vPerpetualAsset.clear();
    vPerpetualPosition.clear();
    vCrossMarginAsset.clear();
}

void GateioAdapterItem::UpdateAccountInfo() {
    std::lock_guard<std::mutex> lk(mut);
    query = true;
    vQueryErrMsg.clear();

    if (spotEnable && gateioSpot) {
        vSpotAsset.clear();
        vector<string> vSpotErrMsg;
        bool spotQueryAccount = gateioSpot->QueryAccount(vSpotAsset, vSpotErrMsg);
        query = query && spotQueryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vSpotErrMsg.begin(), vSpotErrMsg.end());
    }

    if (deliveryEnable && gateioDelivery) {
        vDeliveryAsset.clear();
        vDeliveryPosition.clear();
        vDeliveryOpenOrder.clear();

        vector<string> vDeliveryAssetErrMsg;
        bool deliveryQueryAccount = gateioDelivery->QueryAccount(vDeliveryAsset, vDeliveryAssetErrMsg);
        query = query && deliveryQueryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vDeliveryAssetErrMsg.begin(), vDeliveryAssetErrMsg.end());

        vector<string> vDeliveryPositionErrMsg;
        bool deliveryQueryPosition = gateioDelivery->QueryPosition(vDeliveryPosition, vDeliveryPositionErrMsg);
        query = query && deliveryQueryPosition;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vDeliveryPositionErrMsg.begin(), vDeliveryPositionErrMsg.end());

        vector<string> vDeliveryOpenOrderErrMsg;
        bool deliveryQueryOpenOrder = gateioDelivery->QueryOpenOrder(vDeliveryOpenOrder, vDeliveryOpenOrderErrMsg);
        query = query && deliveryQueryOpenOrder;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vDeliveryOpenOrderErrMsg.begin(), vDeliveryOpenOrderErrMsg.end());
    }


    if (perpetualEnable && gateioPerpetual) {
        vPerpetualAsset.clear();
        vPerpetualPosition.clear();
        vPerpetualOpenOrder.clear();

        vector<string> vPerpetualAssetErrMsg;
        bool perpetualQueryAccount = gateioPerpetual->QueryAccount(vPerpetualAsset, vPerpetualAssetErrMsg);
        query = query && perpetualQueryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vPerpetualAssetErrMsg.begin(), vPerpetualAssetErrMsg.end());

        vector<string> vPerpetualPositionErrMsg;
        bool perpetualQueryPosition = gateioPerpetual->QueryPosition(vPerpetualPosition, vPerpetualPositionErrMsg);
        query = query && perpetualQueryPosition;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vPerpetualPositionErrMsg.begin(), vPerpetualPositionErrMsg.end());

        vector<string> vPerpetualOpenOrderErrMsg;
        bool perpetualQueryOpenOrder = gateioPerpetual->QueryOpenOrder(vPerpetualOpenOrder, vPerpetualOpenOrderErrMsg);
        query = query && perpetualQueryOpenOrder;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vPerpetualOpenOrderErrMsg.begin(), vPerpetualOpenOrderErrMsg.end());
    }

    if (crossMarginEnable && gateioCrossMargin) {
        vCrossMarginAsset.clear();

        vector<string> vCrossMarginAssetErrMsg;
        bool crossMarginQueryAccount = gateioCrossMargin->QueryAccount(vCrossMarginAsset, crossMarginAccountTotal, vCrossMarginAssetErrMsg);
        query = query && crossMarginQueryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vCrossMarginAssetErrMsg.begin(), vCrossMarginAssetErrMsg.end());

    }
    updateTime = GetCurrentTimeUs();
}

set<string> GateioAdapterItem::GetInstrumentList() {
    set<string> s;
    if (baseAsset != "USDT") {
        string instrumentKey = "GATEIO|" + baseAsset + "_USDT" + "|SPOT";
        s.insert(instrumentKey);
    }

    for (size_t i = 0; i < vSpotAsset.size(); ++i) {
        string asset = vSpotAsset[i].currency;
        if (asset != baseAsset && asset != "USDT") {
            string instrumentKey = "GATEIO|" + asset + "_" + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "GATEIO|" + asset + "_USDT" + "|SPOT";
            s.insert(instrumentKey);
        }
    }

    for (size_t i = 0; i < vDeliveryAsset.size(); ++i) {
        string asset = vDeliveryAsset[i].currency;
        if (asset != baseAsset && asset != "USDT") {
            string instrumentKey = "GATEIO|" + asset + "_" + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "GATEIO|" + asset + "_USDT" + "|SPOT";
            s.insert(instrumentKey);
        }
    }
    
    for (size_t i = 0; i < vPerpetualAsset.size(); ++i) {
        string asset = vPerpetualAsset[i].currency;
        if (asset != baseAsset && asset != "USDT") {
            string instrumentKey = "GATEIO|" + asset + "_" + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "GATEIO|" + asset + "_USDT" + "|SPOT";
            s.insert(instrumentKey);
        }
    }
    
    for (size_t i = 0; i < vCrossMarginAsset.size(); ++i) {
        string asset = vCrossMarginAsset[i].currency;
        if (asset != baseAsset && asset != "USDT") {
            string instrumentKey = "GATEIO|" + asset + "_" + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "GATEIO|" + asset + "_USDT" + "|SPOT";
            s.insert(instrumentKey);
        }
    }

    for (size_t i = 0; i < vDeliveryPosition.size(); ++i) {
        string instrumentKey = "GATEIO|" + vDeliveryPosition[i].contract + "|FUTURES";
        s.insert(instrumentKey);
    }

    for (size_t i = 0; i < vPerpetualPosition.size(); ++i) {
        string instrumentKey = "GATEIO|" + vPerpetualPosition[i].contract + "|FUTURES";
        s.insert(instrumentKey);
    }

    return s;
}

vector<gateio::SpotAsset>& GateioAdapterItem::GetSpotAsset() {
    return vSpotAsset;
}

vector<gateio::FutureAsset>& GateioAdapterItem::GetDeliveryAsset() {
    return vDeliveryAsset;
}

vector<gateio::FuturePosition>& GateioAdapterItem::GetDeliveryPosition() {
    return vDeliveryPosition;
}

vector<gateio::FutureOrder>& GateioAdapterItem::GetDeliveryOpenOrder() {
    return vDeliveryOpenOrder;
}

vector<gateio::FutureAsset>& GateioAdapterItem::GetPerpetualAsset() {
    return vPerpetualAsset;
}

vector<gateio::FuturePosition>& GateioAdapterItem::GetPerpetualPosition() {
    return vPerpetualPosition;
}

vector<gateio::FutureOrder>& GateioAdapterItem::GetPerpetualOpenOrder() {
    return vPerpetualOpenOrder;
}

vector<gateio::CrossMarginAsset>& GateioAdapterItem::GetCrossMarginAsset() {
    return vCrossMarginAsset;
}

gateio::CrossMarginAccountTotal& GateioAdapterItem::GetCrossMarginAccountTotal() {
    return crossMarginAccountTotal;
}

double GateioAdapterItem::GetDeliveryPositionValue(string asset) {
    double positionValue = 0.0;
    for (size_t i = 0; i < vDeliveryPosition.size(); ++i) {
        string instrumentKey = "GATEIO|" + vDeliveryPosition[i].contract + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0) {
            if (info.instrumentType == "InstType_USDT_FUTURES" || info.calculateType == 0) {
                positionValue += fabs(vDeliveryPosition[i].size * price * info.multiple);
            } else if (info.instrumentType == "InstType_BTC_FUTURES" || info.calculateType == 1) {
                positionValue += fabs(vDeliveryPosition[i].size / price * info.multipleVolume);
            }
        }
    }
    return positionValue;
}

double GateioAdapterItem::GetPerpetualPositionValue(string asset) {
    double positionValue = 0.0;
    for (size_t i = 0; i < vPerpetualPosition.size(); ++i) {
        string instrumentKey = "GATEIO|" + vPerpetualPosition[i].contract + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        LOG_INFO("GetPerpetualPositionValue: asset:%s  info.margin:%s  price:%f", asset.c_str(), info.margin.c_str(), price);
        if (asset == info.margin && price > 0.0) {
            if (info.instrumentType == "InstType_USDT_SWAP" || info.calculateType == 0) {
                positionValue += fabs(vPerpetualPosition[i].size * price * info.multiple);
            } else if (info.instrumentType == "InstType_BTC_SWAP" || info.calculateType == 1) {
                positionValue += fabs(vPerpetualPosition[i].size / price * info.multipleVolume);
            }
        }
    }
    return positionValue;
}

double GateioAdapterItem::GetDeliveryFloatAmount(string asset) {
    double floatAmount = 0.0;
    for (size_t i = 0; i < vDeliveryPosition.size(); ++i) {
        string instrumentKey = "GATEIO|" + vDeliveryPosition[i].contract + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0 && vDeliveryPosition[i].entryPrice > 0.0) {
            if (info.instrumentType == "InstType_USDT_FUTURES" || info.calculateType == 0) {
                floatAmount += (price - vDeliveryPosition[i].entryPrice) * vDeliveryPosition[i].size * info.multiple;
            } else if (info.instrumentType == "InstType_BTC_FUTURES" || info.calculateType == 1) {
                floatAmount += (1 / vDeliveryPosition[i].entryPrice - 1 / price) * vDeliveryPosition[i].size * info.multipleVolume;
            }
        }
    }
    return floatAmount;
}

double GateioAdapterItem::GetPerpetualFloatAmount(string asset) {
    double floatAmount = 0.0;
    for (size_t i = 0; i < vPerpetualPosition.size(); ++i) {
        string instrumentKey = "GATEIO|" + vPerpetualPosition[i].contract + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0 && vPerpetualPosition[i].entryPrice > 0.0) {
            if (info.instrumentType == "InstType_USDT_SWAP" || info.calculateType == 0) {
                double f = (price - vPerpetualPosition[i].entryPrice) * vPerpetualPosition[i].size * info.multiple;
                floatAmount += f;
                stringstream ss;
                ss << "accountId:" << accountInfo.accountId << " instrumentKey:" << instrumentKey << " price:" << price << " entryPrice:" << vPerpetualPosition[i].entryPrice << " size:" << vPerpetualPosition[i].size << " multiple:" << info.multiple << " floatAmount:" << f;
                LOG_INFO("GetPerpetualFloatAmount: %s", ss.str().c_str()); 
            } else if (info.instrumentType == "InstType_BTC_SWAP" || info.calculateType == 1) {
                floatAmount += (1 / vPerpetualPosition[i].entryPrice - 1 / price) * vPerpetualPosition[i].size * info.multipleVolume;
            }
        }
    }
    return floatAmount;
}

double GateioAdapterItem::GetPerpetualAssetTotal(string asset) {
    double total = 0.0;
    for (size_t i = 0; i < vPerpetualAsset.size(); ++i) {
        if (asset == vPerpetualAsset[i].currency) {
            total = vPerpetualAsset[i].total;
            break;
        }
    }
    return total;
}

void GateioAdapterItem::GetDeliveryLongShortFrozenPosition(string symbol, double longFrozenPos, double shortFrozenPos) {
    for (size_t i = 0; i < vDeliveryOpenOrder.size(); ++i) {
        if (vDeliveryOpenOrder[i].contract == symbol) {
            if (vDeliveryOpenOrder[i].size > 0) {
                longFrozenPos += vDeliveryOpenOrder[i].left;
            } else {
                shortFrozenPos += vDeliveryOpenOrder[i].left;
            }
        }
    }
}

void GateioAdapterItem::GetPerpetualLongShortFrozenPosition(string symbol, double longFrozenPos, double shortFrozenPos) {
    for (size_t i = 0; i < vPerpetualOpenOrder.size(); ++i) {
        if (vPerpetualOpenOrder[i].contract == symbol) {
            if (vPerpetualOpenOrder[i].size > 0) {
                longFrozenPos += vPerpetualOpenOrder[i].left;
            } else {
                shortFrozenPos += vPerpetualOpenOrder[i].left;
            }
        }
    }
}

int64_t GateioAdapterItem::GetUpdateTime() {
    return updateTime;
}

bool GateioAdapterItem::GetQueryStatus() {
    return query;
}

vector<string>& GateioAdapterItem::GetQueryErrMsg() {
    return vQueryErrMsg;
}

int GateioAdapterItem::isUnified() {
    return accountInfo.unified;
}
