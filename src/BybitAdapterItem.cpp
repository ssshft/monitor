#include "BybitAdapterItem.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"


BybitAdapterItem::BybitAdapterItem(AccountInfo info, sm::SecurityManager* s) {
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

    smc = s;
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

        std::vector<std::string> vErrMsg;
        bool queryAccount = byb->QueryAccount(totalAccountInfo, vAsset, vErrMsg);
        query = query && queryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vErrMsg.begin(), vErrMsg.end());

        std::vector<std::string> vPositionErrMsg;
        bool queryPosition = byb->QueryPosition(vPosition, vPositionErrMsg);
        query = query && queryPosition;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vPositionErrMsg.begin(), vPositionErrMsg.end());

        std::vector<std::string> vOpenOrderErrMsg;
        bool queryOpenOrder = byb->QueryOpenOrder(vOpenOrder, vOpenOrderErrMsg);
        query = query && queryOpenOrder;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vOpenOrderErrMsg.begin(), vOpenOrderErrMsg.end());
    }

    updateTime = GetCurrentTimeUs();
}

std::unordered_map<std::string, md::InstrumentInfo> BybitAdapterItem::GetInstrumentList() {
    mInst.clear();
  
    if (baseAsset != "USDT") {
        std::string originInstId = baseAsset + "_USDT";
        md::InstrumentInfo info;
        if (smc->get_instrument_info(BYBIT, SPOT, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, SPOT, info.instId);
            mInst[key] = info;
        }
    }

    for (size_t i = 0; i < vAsset.size(); ++i) {
        string asset = vAsset[i].coin;
        if (asset != baseAsset && asset != "USDT") {
            std::string originInstId = asset + "_" + baseAsset;
            md::InstrumentInfo info;
            if (smc->get_instrument_info(BYBIT, SPOT, originInstId.c_str(), info)) {
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, SPOT, info.instId);
                mInst[key] = info;
            }

            std::string originInstIdUsdt = asset + "_USDT";
            md::InstrumentInfo infoUsdt;
            if (smc->get_instrument_info(BYBIT, SPOT, originInstIdUsdt.c_str(), infoUsdt)) {
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, SPOT, infoUsdt.instId);
                mInst[key] = infoUsdt;
            }
        }
    }

    for (size_t i = 0; i < vPosition.size(); ++i) {
        std::string originInstId = vPosition[i].symbol;
        std::string category = vPosition[i].category;
        md::InstrumentInfo info;
        if (category == "linear") {
            if (smc->get_instrument_info(BYBIT, USDT_SWAP, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, USDT_SWAP, info.instId);
                mInst[key] = info;
            }
            else if (smc->get_instrument_info(BYBIT, USDT_FUTURES, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, USDT_FUTURES, info.instId);
                mInst[key] = info;
            }
        } else if (category == "inverse") {
            if (smc->get_instrument_info(BYBIT, C_SWAP, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, C_SWAP, info.instId);
                mInst[key] = info;
            }
            else if (smc->get_instrument_info(BYBIT, C_FUTURES, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, C_FUTURES, info.instId);
                mInst[key] = info;
            }
        }
    }

    return mInst;
}

bybit::TotalAccountInfo& BybitAdapterItem::GetTotalAccountInfo() {
    return totalAccountInfo;
}

std::vector<bybit::Asset>& BybitAdapterItem::GetAsset() {
    return vAsset;
}

std::vector<bybit::Position>& BybitAdapterItem::GetPosition() {
    return vPosition;
}

std::vector<bybit::Order>& BybitAdapterItem::GetOpenOrder() {
    return vOpenOrder;
}

double BybitAdapterItem::GetPositionValue(std::string asset) {
    double positionValue = 0.0;
    for (size_t i = 0; i < vPosition.size(); ++i) {
        std::string originInstId = vPosition[i].symbol;
        std::string category = vPosition[i].category;
        double price = -1.0;
        md::InstrumentInfo info;
        if (category == "linear") {
            if (smc->get_instrument_info(BYBIT, USDT_SWAP, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, USDT_SWAP, info.instId);
                price = MdMgr::GetInstance().GetMidPrice(key);
            }
            else if (smc->get_instrument_info(BYBIT, USDT_FUTURES, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, USDT_FUTURES, info.instId);
                price = MdMgr::GetInstance().GetMidPrice(key);
            }
        } 
        else if (category == "inverse") {
            if (smc->get_instrument_info(BYBIT, C_SWAP, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, C_SWAP, info.instId);
                price = MdMgr::GetInstance().GetMidPrice(key);
            }
            else if (smc->get_instrument_info(BYBIT, C_FUTURES, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, C_FUTURES, info.instId);
                price = MdMgr::GetInstance().GetMidPrice(key);
            }
        }

        if (asset == info.margin && price > 0.0) {
            if (info.instTypeEnum == USDT_SWAP || info.instTypeEnum == USDT_FUTURES) {
                positionValue += fabs(vPosition[i].size * price * info.value);
            } 
            else if (info.instTypeEnum == C_SWAP || info.instTypeEnum == C_FUTURES) {
                positionValue += fabs(vPosition[i].size / price * info.value);
            }
        }
    }
    return positionValue;
}

double BybitAdapterItem::GetFloatAmount(string asset) {
    double floatAmount = 0.0;
    for (size_t i = 0; i < vPosition.size(); ++i) {
        std::string originInstId = vPosition[i].symbol;
        std::string category = vPosition[i].category;
        double price = -1.0;
        md::InstrumentInfo info;
        if (category == "linear") {
            if (smc->get_instrument_info(BYBIT, USDT_SWAP, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, USDT_SWAP, info.instId);
                price = MdMgr::GetInstance().GetMidPrice(key);
            }
            else if (smc->get_instrument_info(BYBIT, USDT_FUTURES, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, USDT_FUTURES, info.instId);
                price = MdMgr::GetInstance().GetMidPrice(key);
            }
        } 
        else if (category == "inverse") {
            if (smc->get_instrument_info(BYBIT, C_SWAP, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, C_SWAP, info.instId);
                price = MdMgr::GetInstance().GetMidPrice(key);
            }
            else if (smc->get_instrument_info(BYBIT, C_FUTURES, originInstId.c_str(), info)) { 
                std::string key = crypto::get_instrumentInfo_channel_key(BYBIT, C_FUTURES, info.instId);
                price = MdMgr::GetInstance().GetMidPrice(key);
            }
        }

        if (asset == info.margin && price > 0.0 && vPosition[i].avgPrice > 0.0) {
            if (info.instTypeEnum == USDT_SWAP || info.instTypeEnum == USDT_FUTURES) {
                floatAmount += (price - vPosition[i].avgPrice) * vPosition[i].size * info.value;
            } 
            else if (info.instTypeEnum == C_SWAP || info.instTypeEnum == C_FUTURES) {
                floatAmount += (1 / vPosition[i].avgPrice - 1 / price) * vPosition[i].size * info.value;
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
