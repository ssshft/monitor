#include "CoinbaseAdapterMgr.h"


CoinbaseAdapterMgr::CoinbaseAdapterMgr() {
    unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        if (iter->second.exchangeType == ET_COINBASE) {
            mCoinbaseAdapterItem[iter->first] = new CoinbaseAdapterItem(iter->second);
        }
    }
}

CoinbaseAdapterMgr::~CoinbaseAdapterMgr() {
    for (auto iter = mCoinbaseAdapterItem.begin(); iter != mCoinbaseAdapterItem.end(); ++iter) {
        delete iter->second;
        iter->second = nullptr;
    }
    mCoinbaseAdapterItem.clear();
}

CoinbaseAdapterMgr& CoinbaseAdapterMgr::GetInstance() {
	static CoinbaseAdapterMgr coinbaseAdapterMgr;
	return coinbaseAdapterMgr;
}

void CoinbaseAdapterMgr::UpdateAccountInfo() {
    for (auto iter = mCoinbaseAdapterItem.begin(); iter != mCoinbaseAdapterItem.end(); ++iter) {
	bool flag = MonitorConfig::GetInstance().IsAccountIdInProduct(iter->first);
	if (!flag) {
            iter->second->UpdateAccountInfo();
            usleep(500000);
	}
    }
}

set<string> CoinbaseAdapterMgr::GetInstrumentList() {
    set<string> s;
    for (auto iter = mCoinbaseAdapterItem.begin(); iter != mCoinbaseAdapterItem.end(); ++iter) {
        set<string> sItem = iter->second->GetInstrumentList();
        s.insert(sItem.begin(), sItem.end());
    }
    return s;
}

CoinbaseAdapterItem* CoinbaseAdapterMgr::GetAdapterItem(int accountId) {
    CoinbaseAdapterItem* item = nullptr;
    auto iter = mCoinbaseAdapterItem.find(accountId);
    if (iter != mCoinbaseAdapterItem.end()) {
        item = iter->second;
    }
    return item;
}
