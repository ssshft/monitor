#include "BybitAdapterMgr.h"


BybitAdapterMgr::BybitAdapterMgr() {
    unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        if (iter->second.exchangeType == ET_BYBIT) {
            mBybitAdapterItem[iter->first] = new BybitAdapterItem(iter->second);
        }
    }
}

BybitAdapterMgr::~BybitAdapterMgr() {
    for (auto iter = mBybitAdapterItem.begin(); iter != mBybitAdapterItem.end(); ++iter) {
        delete iter->second;
        iter->second = nullptr;
    }
    mBybitAdapterItem.clear();
}

BybitAdapterMgr& BybitAdapterMgr::GetInstance() {
	static BybitAdapterMgr bybitAdapterMgr;
	return bybitAdapterMgr;
}

void BybitAdapterMgr::UpdateAccountInfo() {
    for (auto iter = mBybitAdapterItem.begin(); iter != mBybitAdapterItem.end(); ++iter) {
	    bool flag = MonitorConfig::GetInstance().IsAccountIdInProduct(iter->first);
	    if (!flag) {
            iter->second->UpdateAccountInfo();
	    }
    }
}

set<string> BybitAdapterMgr::GetInstrumentList() {
    set<string> s;
    for (auto iter = mBybitAdapterItem.begin(); iter != mBybitAdapterItem.end(); ++iter) {
        set<string> sItem = iter->second->GetInstrumentList();
        s.insert(sItem.begin(), sItem.end());
    }
    return s;
}

BybitAdapterItem* BybitAdapterMgr::GetAdapterItem(int accountId) {
    BybitAdapterItem* item = nullptr;
    auto iter = mBybitAdapterItem.find(accountId);
    if (iter != mBybitAdapterItem.end()) {
        item = iter->second;
    }
    return item;
}
