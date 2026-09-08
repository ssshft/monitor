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

std::unordered_map<std::string, md::InstrumentInfo> BybitAdapterMgr::GetInstrumentList() {
    std::unordered_map<std::string, md::InstrumentInfo> m;
    for (auto iter = mBybitAdapterItem.begin(); iter != mBybitAdapterItem.end(); ++iter) {
        std::unordered_map<std::string, md::InstrumentInfo> mItem = iter->second->GetInstrumentList();
        for (auto& [key, value] : mItem) {
            m[key] = value;
        }
    }
    return m;
}

BybitAdapterItem* BybitAdapterMgr::GetAdapterItem(int accountId) {
    BybitAdapterItem* item = nullptr;
    auto iter = mBybitAdapterItem.find(accountId);
    if (iter != mBybitAdapterItem.end()) {
        item = iter->second;
    }
    return item;
}
