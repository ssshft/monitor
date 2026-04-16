#include "BinanceAdapterMgr.h"


BinanceAdapterMgr::BinanceAdapterMgr() {
    unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        if (iter->second.exchangeType == BINANCE) {
            mBinanceAdapterItem[iter->first] = new BinanceAdapterItem(iter->second);
        }
    }
}

BinanceAdapterMgr::~BinanceAdapterMgr() {
    for (auto iter = mBinanceAdapterItem.begin(); iter != mBinanceAdapterItem.end(); ++iter) {
        delete iter->second;
        iter->second = nullptr;
    }
    mBinanceAdapterItem.clear();
}

BinanceAdapterMgr& BinanceAdapterMgr::GetInstance() {
	static BinanceAdapterMgr binanceAdapterMgr;
	return binanceAdapterMgr;
}

void BinanceAdapterMgr::UpdateAccountInfo() {
    for (auto iter = mBinanceAdapterItem.begin(); iter != mBinanceAdapterItem.end(); ++iter) {
	bool flag = MonitorConfig::GetInstance().IsAccountIdInProduct(iter->first);
	if (!flag) {
            iter->second->UpdateAccountInfo();
            sleep(4);
	    }
    }
}

set<string> BinanceAdapterMgr::GetInstrumentList() {
    set<string> s;
    for (auto iter = mBinanceAdapterItem.begin(); iter != mBinanceAdapterItem.end(); ++iter) {
        set<string> sItem = iter->second->GetInstrumentList();
        s.insert(sItem.begin(), sItem.end());
    }
    return s;
}

BinanceAdapterItem* BinanceAdapterMgr::GetAdapterItem(int accountId) {
    BinanceAdapterItem* item = nullptr;
    auto iter = mBinanceAdapterItem.find(accountId);
    if (iter != mBinanceAdapterItem.end()) {
        item = iter->second;
    }
    return item;
}
