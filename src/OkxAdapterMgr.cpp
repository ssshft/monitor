#include "OkxAdapterMgr.h"


OkxAdapterMgr::OkxAdapterMgr() {
    unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        if (iter->second.exchangeType == ET_OKX) {
            mOkxAdapterItem[iter->first] = new OkxAdapterItem(iter->second);
        }
    }
}

OkxAdapterMgr::~OkxAdapterMgr() {
    for (auto iter = mOkxAdapterItem.begin(); iter != mOkxAdapterItem.end(); ++iter) {
        delete iter->second;
        iter->second = nullptr;
    }
    mOkxAdapterItem.clear();
}

OkxAdapterMgr& OkxAdapterMgr::GetInstance() {
	static OkxAdapterMgr okxAdapterMgr;
	return okxAdapterMgr;
}

void OkxAdapterMgr::UpdateAccountInfo() {
    for (auto iter = mOkxAdapterItem.begin(); iter != mOkxAdapterItem.end(); ++iter) {
	    bool flag = MonitorConfig::GetInstance().IsAccountIdInProduct(iter->first);
	    if (!flag) {
            iter->second->UpdateAccountInfo();
	    }
    }
}

set<string> OkxAdapterMgr::GetInstrumentList() {
    set<string> s;
    for (auto iter = mOkxAdapterItem.begin(); iter != mOkxAdapterItem.end(); ++iter) {
        set<string> sItem = iter->second->GetInstrumentList();
        s.insert(sItem.begin(), sItem.end());
    }
    return s;
}

OkxAdapterItem* OkxAdapterMgr::GetAdapterItem(int accountId) {
    OkxAdapterItem* item = nullptr;
    auto iter = mOkxAdapterItem.find(accountId);
    if (iter != mOkxAdapterItem.end()) {
        item = iter->second;
    }
    return item;
}
