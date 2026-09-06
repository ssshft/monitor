#include "BybitAdapterMgr.h"


BybitAdapterMgr::BybitAdapterMgr() {

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

void BybitAdapterMgr::Init(sm::SecurityManager* s) {
    std::unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        if (iter->second.exchangeType == BYBIT) {
            mBybitAdapterItem[iter->first] = new BybitAdapterItem(iter->second, s);
        }
    }
}

void BybitAdapterMgr::UpdateAccountInfo() {
    for (auto iter = mBybitAdapterItem.begin(); iter != mBybitAdapterItem.end(); ++iter) {
	    bool flag = MonitorConfig::GetInstance().IsAccountIdInProduct(iter->first);
	    if (!flag) {
            iter->second->UpdateAccountInfo();
            sleep(1);
	    }
    }
}

std::set<std::string> BybitAdapterMgr::GetInstrumentList() {
    std::set<std::string> s;
    for (auto iter = mBybitAdapterItem.begin(); iter != mBybitAdapterItem.end(); ++iter) {
        std::set<std::string> sItem = iter->second->GetInstrumentList();
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
