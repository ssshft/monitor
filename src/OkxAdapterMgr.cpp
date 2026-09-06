#include "OkxAdapterMgr.h"


OkxAdapterMgr::OkxAdapterMgr() {

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

void OkxAdapterMgr::Init(sm::SecurityManager* s) {
    std::unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        if (iter->second.exchangeType == OKX) {
            mOkxAdapterItem[iter->first] = new OkxAdapterItem(iter->second, s);
        }
    }
}

void OkxAdapterMgr::UpdateAccountInfo() {
    for (auto iter = mOkxAdapterItem.begin(); iter != mOkxAdapterItem.end(); ++iter) {
	    bool flag = MonitorConfig::GetInstance().IsAccountIdInProduct(iter->first);
	    if (!flag) {
            iter->second->UpdateAccountInfo();
            sleep(1);
	    }
    }
}

std::set<std::string> OkxAdapterMgr::GetInstrumentList() {
    std::set<std::string> s;
    for (auto iter = mOkxAdapterItem.begin(); iter != mOkxAdapterItem.end(); ++iter) {
        std::set<std::string> sItem = iter->second->GetInstrumentList();
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
