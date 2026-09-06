#include "OkxAdapterItem.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"


OkxAdapterItem::OkxAdapterItem(AccountInfo info, sm::SecurityManager* s) {
    accountInfo = info;
    okxClient = new OkxClient(accountInfo);
    baseAsset = accountInfo.baseAsset;
    updateTime = 0;
    std::string apiPermission = accountInfo.apiPermission;

    smc = s;
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
        vOrder.clear();

        std::vector<std::string> vAssetErrMsg;
        bool queryAccount = okxClient->QueryAccount(vAsset, vAssetErrMsg);
        query = query && queryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vAssetErrMsg.begin(), vAssetErrMsg.end());

        std::vector<std::string> vPositionErrMsg;
        bool queryPosition = okxClient->QueryPosition(vPosition, vPositionErrMsg);
        query = query && queryPosition;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vPositionErrMsg.begin(), vPositionErrMsg.end());

        std::vector<std::string> vOpenOrderErrMsg;
        bool queryOpenOrder = okxClient->QueryOpenOrder(vOpenOrder, vOpenOrderErrMsg);
        query = query && queryOpenOrder;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vOpenOrderErrMsg.begin(), vOpenOrderErrMsg.end());

        std::vector<std::string> vErr;
        okxClient->QueryOrder(vOrder, vErr);
    }

    updateTime = GetCurrentTimeUs();
}

std::unordered_map<std::stirng, md::InstrumentInfo> OkxAdapterItem::GetInstrumentList() {
    mInst.clear();
    if (baseAsset != "USDT") {
        std::string originInstId = baseAsset + "-USDT";
        md::InstrumentInfo info;
        if (smc->get_instrument_info(OKX, SPOT, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(OKX, SPOT, info.instId);
            mInst[key] = info;
        }
    }

    for (size_t i = 0; i < vAsset.size(); ++i) {
        string asset = vAsset[i].ccy;
        if (asset != baseAsset && asset != "USDT") {
            std::string originInstId = asset + "-" + baseAsset;
            md::InstrumentInfo info;
            if (smc->get_instrument_info(OKX, SPOT, originInstId.c_str(), info)) {
                std::string key = crypto::get_instrumentInfo_channel_key(OKX, SPOT, info.instId);
                mInst[key] = info;
            }

            std::string originInstIdUsdt = asset + "-USDT";
            md::InstrumentInfo infoUsdt;
            if (smc->get_instrument_info(OKX, SPOT, originInstIdUsdt.c_str(), infoUsdt)) {
                std::string key = crypto::get_instrumentInfo_channel_key(OKX, SPOT, infoUsdt.instId);
                mInst[key] = infoUsdt;
            }
        }
    }

    for (size_t i = 0; i < vPosition.size(); ++i) {
        std::string originInstId = vPosition[i].instId;
        std::string instType = vPosition[i].instType;
        md::InstrumentInfo info;

        if (instType == "SWAP" || instType == "FUTURES") {
            InstType u_swap = (instType == "SWAP") ? USDT_SWAP : USDT_FUTURES;
            InstType c_swap = (instType == "SWAP") ? C_SWAP : C_FUTURES;

            if (smc->get_instrument_info(OKX, u_swap, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(OKX, u_swap, info.instId);
                mInst[key] = info;
            }

            if (smc->get_instrument_info(OKX, c_swap, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(OKX, c_swap, info.instId);
                mInst[key] = info;
            }
        }
    }
    return mInst;
}

std::vector<okx::OkxAsset>& OkxAdapterItem::GetAsset() {
    return vAsset;
}

std::vector<okx::OkxPosition>& OkxAdapterItem::GetPosition() {
    return vPosition;
}

std::vector<okx::OkxOrder>& OkxAdapterItem::GetOpenOrder() {
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

