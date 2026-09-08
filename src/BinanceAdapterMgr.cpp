#include "BinanceAdapterMgr.h"


BinanceAdapterMgr::BinanceAdapterMgr() {

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

void BinanceAdapterMgr::Init(sm::SecurityManager* s) {
    std::unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        if (iter->second.exchangeType == BINANCE) {
            mBinanceAdapterItem[iter->first] = new BinanceAdapterItem(iter->second, s);
        }
    }
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

std::unordered_map<std::string, md::InstrumentInfo>  BinanceAdapterMgr::GetInstrumentList() {
    std::unordered_map<std::string, md::InstrumentInfo> m;
    for (auto iter = mBinanceAdapterItem.begin(); iter != mBinanceAdapterItem.end(); ++iter) {
        std::unordered_map<std::string, md::InstrumentInfo> mItem = iter->second->GetInstrumentList();
        for (auto& [key, value] : mItem) {
            m[key] = value;
        }

    }
    return m;
}

BinanceAdapterItem* BinanceAdapterMgr::GetAdapterItem(int accountId) {
    BinanceAdapterItem* item = nullptr;
    auto iter = mBinanceAdapterItem.find(accountId);
    if (iter != mBinanceAdapterItem.end()) {
        item = iter->second;
    }
    return item;
}
