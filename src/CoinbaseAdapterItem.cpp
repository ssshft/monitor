#include "CoinbaseAdapterItem.h"
#include "BasicInfoMgr.h"


CoinbaseAdapterItem::CoinbaseAdapterItem(AccountInfo info) {
    accountInfo = info;
    coinbaseSpot = new CoinbaseSpot(accountInfo);
    baseAsset = accountInfo.baseAsset;
    updateTime = 0;
    query = true;
    spotEnable = false;
    string apiPermission = accountInfo.apiPermission;
    vector<string> v;
    SplitString(apiPermission, ",", v);
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == "all") {
            spotEnable = true;
        } else if (v[i] == "spot") {
            spotEnable = true;
        }
    }
}

CoinbaseAdapterItem::~CoinbaseAdapterItem() {
    if (coinbaseSpot) {
        delete coinbaseSpot;
        coinbaseSpot = nullptr;
    }

    vSpotAsset.clear();
}

void CoinbaseAdapterItem::UpdateAccountInfo() {
    std::lock_guard<std::mutex> lk(mut);
    query = true;
    vQueryErrMsg.clear();

    if (spotEnable && coinbaseSpot) {
        vSpotAsset.clear();
        
        vector<string> vSpotErrMsg;
        bool querySpotAccount = coinbaseSpot->QueryAccount(vSpotAsset, vSpotErrMsg);
        query = query && querySpotAccount;
        vQueryErrMsg.insert(vQueryErrMsg.end(), vSpotErrMsg.begin(), vSpotErrMsg.end());
    }
    updateTime = GetCurrentTimeUs();
}

set<string> CoinbaseAdapterItem::GetInstrumentList() {
    set<string> s;
    for (size_t i = 0; i < vSpotAsset.size(); ++i) {
        string asset = vSpotAsset[i].symbol;
        if (asset == "USD" || asset == "USDC") {
            continue;
        }
        string product = asset + "-USD";
        s.insert(product);
    }
    return s;
}

vector<coinbase::SpotAsset>& CoinbaseAdapterItem::GetSpotAsset() {
    return vSpotAsset;
}

int64_t CoinbaseAdapterItem::GetUpdateTime() {
    return updateTime;
}

bool CoinbaseAdapterItem::GetQueryStatus() {
    return query;
}

vector<string> CoinbaseAdapterItem::GetQueryErrMsg() {
    return vQueryErrMsg;
}

int CoinbaseAdapterItem::isUnified() {
    return accountInfo.unified;
}