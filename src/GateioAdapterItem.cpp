#include "GateioAdapterItem.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"


GateioAdapterItem::GateioAdapterItem(AccountInfo info, sm::SecurityManager* s) {
    accountInfo = info;
    gateioSpot = new GateioSpot(accountInfo);
    gateioPerpetual = new GateioPerpetual(accountInfo);
    gateioUnified = new GateioUnified(accountInfo);
    baseAsset = accountInfo.baseAsset;
    updateTime = 0;
    spotEnable = false;
    perpetualEnable = false;
    unifiedEnable = false;
    string apiPermission = accountInfo.apiPermission;
    vector<string> v;
    SplitString(apiPermission, ",", v);
    for (size_t i = 0; i < v.size(); ++i) {  // 若是统一账户则无需设置现货的权限，合约的权限需要设置
        if (v[i] == "all") {
            spotEnable = true;
            perpetualEnable = true;
        } else if (v[i] == "spot") {
            spotEnable = true;
        } else if (v[i] == "perpetual") {
            perpetualEnable = true;
        } else if (v[i] == "unified") {
            unifiedEnable = true;
        }
    }

    smc = s;
}

GateioAdapterItem::~GateioAdapterItem() {
    if (gateioSpot) {
        delete gateioSpot;
        gateioSpot = nullptr;
    }
    if (gateioPerpetual) {
        delete gateioPerpetual;
        gateioPerpetual = nullptr;
    }
    if (gateioUnified) {
        delete gateioUnified;
        gateioUnified = nullptr;
    }

    vSpotAsset.clear();
    vPerpetualAsset.clear();
    vPerpetualPosition.clear();
}

void GateioAdapterItem::UpdateAccountInfo() {
    std::lock_guard<std::mutex> lk(mut);
    query = true;
    vQueryErrMsg.clear();

    if (spotEnable && gateioSpot) {
        if (!unifiedEnable) {  // 若是统一账户，则账户信息查询只使用统一账户的查询接口
            vSpotAsset.clear();
            vector<string> vSpotErrMsg;
            bool spotQueryAccount = gateioSpot->QueryAccount(vSpotAsset, vSpotErrMsg);
            query = query && spotQueryAccount;
            vQueryErrMsg.insert(vQueryErrMsg.end(), vSpotErrMsg.begin(), vSpotErrMsg.end());
        }
    }


    if (perpetualEnable && gateioPerpetual) {
        vPerpetualAsset.clear();
        vPerpetualPosition.clear();
        vPerpetualOpenOrder.clear();

        if (!unifiedEnable) {  // 若是统一账户，则账户信息查询只使用统一账户的查询接口
            vector<string> vPerpetualAssetErrMsg;
            bool perpetualQueryAccount = gateioPerpetual->QueryAccount(vPerpetualAsset, vPerpetualAssetErrMsg);
            query = query && perpetualQueryAccount;
            vQueryErrMsg.insert(vQueryErrMsg.end(), vPerpetualAssetErrMsg.begin(), vPerpetualAssetErrMsg.end());
        }

        vector<string> vPerpetualPositionErrMsg;
        bool perpetualQueryPosition = gateioPerpetual->QueryPosition(vPerpetualPosition, vPerpetualPositionErrMsg);
        query = query && perpetualQueryPosition;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vPerpetualPositionErrMsg.begin(), vPerpetualPositionErrMsg.end());

        vector<string> vPerpetualOpenOrderErrMsg;
        bool perpetualQueryOpenOrder = gateioPerpetual->QueryOpenOrder(vPerpetualOpenOrder, vPerpetualOpenOrderErrMsg);
        query = query && perpetualQueryOpenOrder;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vPerpetualOpenOrderErrMsg.begin(), vPerpetualOpenOrderErrMsg.end());

        vector<string> vPerpetualOrderErrMsg;
        bool perpetualQueryOrder = gateioPerpetual->QueryOrder(vPerpetualOrder, vPerpetualOrderErrMsg);
        //query = query && perpetualQueryOrder;
        //vQueryErrMsg.insert(vQueryErrMsg.end(), vPerpetualOrderErrMsg.begin(), vPerpetualOrderErrMsg.end());
    }


    if (unifiedEnable && gateioUnified) {
        vSpotAsset.clear();  // 统一账户暂时使用放到现货里
        vector<string> vUnifiedErrMsg;
        bool unifiedQueryAccount = gateioUnified->QueryAccount(vSpotAsset, vUnifiedErrMsg);
        query = query && unifiedQueryAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vUnifiedErrMsg.begin(), vUnifiedErrMsg.end()); 
    }
    updateTime = GetCurrentTimeUs();
}

std::unordered_map<std::string, md::InstrumentInfo> GateioAdapterItem::GetInstrumentList() {
    mInst.clear();

    set<string> s;
    if (baseAsset != "USDT") {
        std::string originInstId = baseAsset + "_USDT";
        md::InstrumentInfo info;
        if (smc->get_instrument_info(GATEIO, SPOT, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(GATEIO, SPOT, info.instId);
            mInst[key] = info;
        }
    }

    for (size_t i = 0; i < vSpotAsset.size(); ++i) {
        string asset = vSpotAsset[i].currency;
        if (asset != baseAsset && asset != "USDT") {
            std::string originInstId = asset + "_" + baseAsset;
            md::InstrumentInfo info;
            if (smc->get_instrument_info(GATEIO, SPOT, originInstId.c_str(), info)) {
                std::string key = crypto::get_instrumentInfo_channel_key(GATEIO, SPOT, info.instId);
                mInst[key] = info;
            }

            std::string originInstIdUsdt = asset + "_USDT";
            md::InstrumentInfo infoUsdt;
            if (smc->get_instrument_info(GATEIO, SPOT, originInstIdUsdt.c_str(), infoUsdt)) {
                std::string key = crypto::get_instrumentInfo_channel_key(GATEIO, SPOT, infoUsdt.instId);
                mInst[key] = infoUsdt;
            }
        }
    }
    
    for (size_t i = 0; i < vPerpetualAsset.size(); ++i) {
        string asset = vPerpetualAsset[i].currency;
        if (asset != baseAsset && asset != "USDT") {
            std::string originInstId = asset + "_" + baseAsset;
            md::InstrumentInfo info;
            if (smc->get_instrument_info(GATEIO, SPOT, originInstId.c_str(), info)) {
                std::string key = crypto::get_instrumentInfo_channel_key(GATEIO, SPOT, info.instId);
                mInst[key] = info;
            }

            std::string originInstIdUsdt = asset + "_USDT";
            md::InstrumentInfo infoUsdt;
            if (smc->get_instrument_info(GATEIO, SPOT, originInstIdUsdt.c_str(), infoUsdt)) {
                std::string key = crypto::get_instrumentInfo_channel_key(GATEIO, SPOT, infoUsdt.instId);
                mInst[key] = infoUsdt;
            }
        }
    }

    for (size_t i = 0; i < vPerpetualPosition.size(); ++i) {
        std::string originInstId = vPerpetualPosition[i].contract;
        md::InstrumentInfo info;
        if (smc->get_instrument_info(GATEIO, USDT_SWAP, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(GATEIO, USDT_SWAP, info.instId);
            mInst[key] = info;
        }
    }

    return mInst;
}

vector<gateio::SpotAsset>& GateioAdapterItem::GetSpotAsset() {
    return vSpotAsset;
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

vector<gateio::FutureOrder>& GateioAdapterItem::GetPerpetualOrder() {
    return vPerpetualOrder;
}

double GateioAdapterItem::GetPerpetualFloatAmount(string asset) {
    double floatAmount = 0.0;
    for (size_t i = 0; i < vPerpetualPosition.size(); ++i) {
        std::string originInstId = vPerpetualPosition[i].contract;
        double price = -1.0;
        md::InstrumentInfo info;
        if (smc->get_instrument_info(GATEIO, USDT_SWAP, originInstId.c_str(), info)) {
            std::string key = crypto::get_instrumentInfo_channel_key(GATEIO, USDT_SWAP, info.instId);
            price = MdMgr::GetInstance().GetMidPrice(key);
        }

        if (asset == info.margin && price > 0.0 && vPerpetualPosition[i].entryPrice > 0.0) {
            double f = (price - vPerpetualPosition[i].entryPrice) * vPerpetualPosition[i].size * info.value;
            floatAmount += f;
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
