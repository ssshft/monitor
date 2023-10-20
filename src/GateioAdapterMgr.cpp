#include "GateioAdapterMgr.h"


GateioAdapterMgr::GateioAdapterMgr() {
    unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        if (iter->second.exchangeType == ET_GATEIO) {
            mGateioAdapterItem[iter->first] = new GateioAdapterItem(iter->second);
        }
    }
}

GateioAdapterMgr::~GateioAdapterMgr() {
    for (auto iter = mGateioAdapterItem.begin(); iter != mGateioAdapterItem.end(); ++iter) {
        delete iter->second;
        iter->second = nullptr;
    }
    mGateioAdapterItem.clear();
}

GateioAdapterMgr& GateioAdapterMgr::GetInstance() {
	static GateioAdapterMgr gateioAdapterMgr;
	return gateioAdapterMgr;
}

void GateioAdapterMgr::UpdateAccountInfo() {
    for (auto iter = mGateioAdapterItem.begin(); iter != mGateioAdapterItem.end(); ++iter) {
	bool flag = MonitorConfig::GetInstance().IsAccountIdInProduct(iter->first);
	if (!flag) {
            iter->second->UpdateAccountInfo();
	}
    }
}

set<string> GateioAdapterMgr::GetInstrumentList() {
    set<string> s;
    for (auto iter = mGateioAdapterItem.begin(); iter != mGateioAdapterItem.end(); ++iter) {
        set<string> sItem = iter->second->GetInstrumentList();
        s.insert(sItem.begin(), sItem.end());
    }
    return s;
}

GateioAdapterItem* GateioAdapterMgr::GetAdapterItem(int accountId) {
    GateioAdapterItem* item = nullptr;
    auto iter = mGateioAdapterItem.find(accountId);
    if (iter != mGateioAdapterItem.end()) {
        item = iter->second;
    }
    return item;
}
