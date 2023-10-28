#include "OkxAdapterItem.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"


OkxAdapterItem::OkxAdapterItem(AccountInfo info) {
    accountInfo = info;
    okxClient = new OkxClient(accountInfo);
    baseAsset = accountInfo.baseAsset;
    updateTime = 0;
    string apiPermission = accountInfo.apiPermission;
}

OkxAdapterItem::~OkxAdapterItem() {
    if (okxClient) {
        delete okxClient;
        okxClient = nullptr;
    }

    vAsset.clear();
    vPosition.clear();
    vOpenOrder.clear();
}

void OkxAdapterItem::UpdateAccountInfo() {
    std::lock_guard<std::mutex> lk(mut);
    query = true;
    vQueryErrMsg.clear();

    if (okxClient) {
        vAsset.clear();
        vPosition.clear();
        vOpenOrder.clear();

        vector<string> vAssetErrMsg;
        bool queryAccount = okxClient->QueryAccount(vAsset, vAssetErrMsg);
        query = query && queryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vAssetErrMsg.begin(), vAssetErrMsg.end());

        vector<string> vPositionErrMsg;
        bool queryPosition = okxClient->QueryPosition(vPosition, vPositionErrMsg);
        query = query && queryPosition;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vPositionErrMsg.begin(), vPositionErrMsg.end());

        vector<string> vOpenOrderErrMsg;
        bool queryOpenOrder = okxClient->QueryOpenOrder(vOpenOrder, vOpenOrderErrMsg);
        query = query && queryOpenOrder;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vOpenOrderErrMsg.begin(), vOpenOrderErrMsg.end());
    }

    updateTime = GetCurrentTimeUs();
}

set<string> OkxAdapterItem::GetInstrumentList() {
    set<string> s;
    if (baseAsset != "USDT") {
        string instrumentKey = "OKX|" + baseAsset + "-USDT" + "|SPOT";
        s.insert(instrumentKey);
    }

    for (size_t i = 0; i < vAsset.size(); ++i) {
        string asset = vAsset[i].ccy;
        if (asset != baseAsset && asset != "USDT") {
            string instrumentKey = "OKX|" + asset + "-" + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = "OKX|" + asset + "-USDT" + "|SPOT";
            s.insert(instrumentKey);
        }
    }

    for (size_t i = 0; i < vPosition.size(); ++i) {
        string instrumentKey = "OKX|" + vPosition[i].instId + "|FUTURES";
        s.insert(instrumentKey);
    }
    return s;
}

vector<okx::OkxAsset>& OkxAdapterItem::GetAsset() {
    return vAsset;
}

vector<okx::OkxPosition>& OkxAdapterItem::GetPosition() {
    return vPosition;
}

vector<okx::OkxOrder>& OkxAdapterItem::GetOpenOrder() {
    return vOpenOrder;
}

double OkxAdapterItem::GetPositionValue(string asset) {
    double positionValue = 0.0;
    for (size_t i = 0; i < vPosition.size(); ++i) {
        string instrumentKey = "OKX|" + vPosition[i].instId + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0) {
            if (info.instrumentType == "InstType_USDT_FUTURES" || info.instrumentType == "InstType_USDT_SWAP" || info.calculateType == 0) {
                positionValue += fabs(vPosition[i].pos * price * info.multiple);
            } else if (info.instrumentType == "InstType_C_FUTURES" || info.instrumentType == "InstType_C_SWAP" || info.calculateType == 1) {
                positionValue += fabs(vPosition[i].pos / price * info.multiple);
            }
        }
    }
    return positionValue;
}

double OkxAdapterItem::GetFloatAmount(string asset) {
    double floatAmount = 0.0;
    for (size_t i = 0; i < vPosition.size(); ++i) {
        string instrumentKey = "OKX|" + vPosition[i].instId + "|FUTURES";
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
        double price = BinanceMdMgr::GetInstance().GetMidPrice(key);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        if (asset == info.margin && price > 0.0 && vPosition[i].avgPx > 0.0) {
            if (info.instrumentType == "InstType_USDT_FUTURES" || info.instrumentType == "InstType_USDT_SWAP" || info.calculateType == 0) {
                floatAmount += (price - vPosition[i].avgPx) * vPosition[i].pos * info.multiple;
            } else if (info.instrumentType == "InstType_C_FUTURES" || info.instrumentType == "InstType_C_SWAP" || info.calculateType == 1) {
                floatAmount += (1 / vPosition[i].avgPx - 1 / price) * vPosition[i].pos * info.multiple;
            }
        }
    }
    return floatAmount;
}

void OkxAdapterItem::GetLongShortFrozenPosition(string symbol, double longFrozenPos, double shortFrozenPos) {
    for (size_t i = 0; i < vOpenOrder.size(); ++i) {
        if (vOpenOrder[i].instId == symbol) {
            if (vOpenOrder[i].side == "buy") {
                longFrozenPos += vOpenOrder[i].sz - vOpenOrder[i].accFillSz;
            } else {
                shortFrozenPos += vOpenOrder[i].sz - vOpenOrder[i].accFillSz;
            }
        }
    }
}

int64_t OkxAdapterItem::GetUpdateTime() {
    return updateTime;
}

bool OkxAdapterItem::GetQueryStatus() {
    return query;
}

vector<string>& OkxAdapterItem::GetQueryErrMsg() {
    return vQueryErrMsg;
}

int OkxAdapterItem::isUnified() {
    return accountInfo.unified;
}

