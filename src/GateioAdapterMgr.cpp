#include "GateioAdapterMgr.h"


GateioAdapterMgr::GateioAdapterMgr() {

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

void GateioAdapterMgr::Init(sm::SecurityManager* s) {
    std::unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        if (iter->second.exchangeType == GATEIO) {
            mGateioAdapterItem[iter->first] = new GateioAdapterItem(iter->second, s);
        }
    }
}

void GateioAdapterMgr::UpdateAccountInfo() {
    for (auto iter = mGateioAdapterItem.begin(); iter != mGateioAdapterItem.end(); ++iter) {
	bool flag = MonitorConfig::GetInstance().IsAccountIdInProduct(iter->first);
	if (!flag) {
            iter->second->UpdateAccountInfo();
            sleep(1);
	}
    }
}

std::unordered_map<std::string, md::InstrumentInfo> GateioAdapterMgr::GetInstrumentList() {
    std::unordered_map<std::string, md::InstrumentInfo> m;
    for (auto iter = mGateioAdapterItem.begin(); iter != mGateioAdapterItem.end(); ++iter) {
        std::unordered_map<std::string, md::InstrumentInfo> mItem = iter->second->GetInstrumentList();
        for (auto& [key, value] : mItem) {
            m[key] = value;
        }
    }
    return m;
}

GateioAdapterItem* GateioAdapterMgr::GetAdapterItem(int accountId) {
    GateioAdapterItem* item = nullptr;
    auto iter = mGateioAdapterItem.find(accountId);
    if (iter != mGateioAdapterItem.end()) {
        item = iter->second;
    }
    return item;
}
