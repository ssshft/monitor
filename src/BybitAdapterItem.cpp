#include "BybitAdapterItem.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"


BybitAdapterItem::BybitAdapterItem(AccountInfo info) {
    accountInfo = info;
    byb = new Bybit(accountInfo);
    baseAsset = accountInfo.baseAsset;
    updateTime = 0;
    enable = false;

    string apiPermission = accountInfo.apiPermission;
    vector<string> v;
    SplitString(apiPermission, ",", v);
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == "all") {
            enable = true;
        }
    }
}

BybitAdapterItem::~BybitAdapterItem() {
    if (byb) {
        delete byb;
        byb = nullptr;
    }

    vAsset.clear();
    vPosition.clear();
    vOpenOrder.clear();
}

void BybitAdapterItem::UpdateAccountInfo() {
    std::lock_guard<std::mutex> lk(mut);
    query = true;
    vQueryErrMsg.clear();

    if (enable && byb != nullptr) {
        vAsset.clear();
        vPosition.clear();
        vOpenOrder.clear();

        vector<string> vErrMsg;
        bool queryAccount = byb->QueryAccount(totalAccountInfo, vAsset, vErrMsg);
        query = query && queryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vErrMsg.begin(), vErrMsg.end());

        vector<string> vPositionErrMsg;
        bool queryPosition = byb->QueryPosition(vPosition, vPositionErrMsg);
        query = query && queryPosition;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vPositionErrMsg.begin(), vPositionErrMsg.end());

        vector<string> vOpenOrderErrMsg;
        bool queryOpenOrder = byb->QueryOpenOrder(vOpenOrder, vOpenOrderErrMsg);
        query = query && queryOpenOrder;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vOpenOrderErrMsg.begin(), vOpenOrderErrMsg.end());
    }

    updateTime = GetCurrentTimeUs();
}

set<string> BybitAdapterItem::GetInstrumentList() {
    set<string> s;
    if (baseAsset != "USDT") {
        string instrumentKey = "BYBIT|" + baseAsset + "_USDT" + "|SPOT";
        s.insert(instrumentKey);
    }

    for (size_t i = 0; i < vAsset.size(); ++i) {
        string asset = vAsset[i].coin;
        if (asset != baseAsset && asset != "USDT") {
            string instrumentKey = "BYBIT|" + asset + "_" + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "BYBIT|" + asset + "_USDT" + "|SPOT";
            s.insert(instrumentKey);
        }
    }

    for (size_t i = 0; i < vPosition.size(); ++i) {
        string instrumentKey = "BYBIT|" + vPosition[i].symbol + "|FUTURES";
        s.insert(instrumentKey);
    }

    return s;
}

bybit::TotalAccountInfo& BybitAdapterItem::GetTotalAccountInfo() {
    return totalAccountInfo;
}

vector<bybit::Asset>& BybitAdapterItem::GetAsset() {
    return vAsset;
}

vector<bybit::Position>& BybitAdapterItem::GetPosition() {
    return vPosition;
}

vector<bybit::Order>& BybitAdapterItem::GetOpenOrder() {
    return vOpenOrder;
}

double BybitAdapterItem::GetPositionValue(string asset) {
    double positionValue = 0.0;
    for (size_t i = 0; i < vPosition.size(); ++i) {
        string instrumentKey = "BYBIT|" + vPosition[i].symbol + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0) {
            if (info.instrumentType == "InstType_USDT_FUTURES" || info.instrumentType == "InstType_USDT_SWAP" || info.calculateType == 0) {
                positionValue += fabs(vPosition[i].size * price * info.multiple);
            } else if (info.instrumentType == "InstType_C_FUTURES" || info.instrumentType == "InstType_C_SWAP" || info.calculateType == 1) {
                positionValue += fabs(vPosition[i].size / price * info.multiple);
            }
        }
    }
    return positionValue;
}

double BybitAdapterItem::GetFloatAmount(string asset) {
    double floatAmount = 0.0;
    for (size_t i = 0; i < vPosition.size(); ++i) {
        string instrumentKey = "BYBIT|" + vPosition[i].symbol + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0 && vPosition[i].avgPrice > 0.0) {
            if (info.instrumentType == "InstType_USDT_FUTURES" || info.instrumentType == "InstType_USDT_SWAP" || info.calculateType == 0) {
                floatAmount += (price - vPosition[i].avgPrice) * vPosition[i].size * info.multiple;
            } else if (info.instrumentType == "InstType_C_FUTURES" || info.instrumentType == "InstType_C_SWAP" || info.calculateType == 1) {
                floatAmount += (1 / vPosition[i].avgPrice - 1 / price) * vPosition[i].size * info.multiple;
            }
        }
    }
    return floatAmount;
}

void BybitAdapterItem::GetLongShortFrozenPosition(string symbol, double longFrozenPos, double shortFrozenPos) {
    for (size_t i = 0; i < vOpenOrder.size(); ++i) {
        if (vOpenOrder[i].symbol == symbol) {
            if (vOpenOrder[i].side == "Buy") {
                longFrozenPos += vOpenOrder[i].qty - vOpenOrder[i].cumExecQty;
            } else {
                shortFrozenPos += vOpenOrder[i].qty - vOpenOrder[i].cumExecQty;
            }
        }
    }
}

int64_t BybitAdapterItem::GetUpdateTime() {
    return updateTime;
}

bool BybitAdapterItem::GetQueryStatus() {
    return query;
}

vector<string>& BybitAdapterItem::GetQueryErrMsg() {
    return vQueryErrMsg;
}

int BybitAdapterItem::isUnified() {
    return accountInfo.unified;
}
