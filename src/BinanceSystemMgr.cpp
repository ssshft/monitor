#include "BinanceSystemMgr.h"
#include "MonitorConfig.h"


BinanceSystemMgr::BinanceSystemMgr() {
    unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        mBinanceSystemItem[iter->first] = new BinanceSystemItem(iter->first, iter->second.accountName);
    }

    vector<int>& vStrategyAccount = MonitorConfig::GetInstance().GetStrategyAccount();
    for (size_t i = 0; i < vStrategyAccount.size(); ++i) {
        int accountId = vStrategyAccount[i];
        mBinanceSystemItem[accountId] = new BinanceSystemItem(accountId, "");
    }
}

BinanceSystemMgr::~BinanceSystemMgr() {
    for (auto iter = mBinanceSystemItem.begin(); iter != mBinanceSystemItem.end(); ++iter) {
        delete iter->second;
        iter->second = nullptr;
    }
    mBinanceSystemItem.clear();
}

void BinanceSystemMgr::OnSubMessage(const web::json::value& content) {
    int cmdType = content.at("cmd_type").as_integer();
    if (cmdType == ASSET_AND_POSITION_UPDATE_REPORT) {  // 资产持仓更新
        int type = content.at("type").as_integer();
        if (type == ASSET_ALL) {                        // 全量
            int customerId = content.at("customer_id").as_integer();
            auto iter = mBinanceSystemItem.find(customerId);
            if (iter != mBinanceSystemItem.end()) {
                iter->second->OnSubMessage(content);
            }
        }
    }
}

BinanceSystemMgr& BinanceSystemMgr::GetInstance() {
	static BinanceSystemMgr binanceSystemMgr;
	return binanceSystemMgr;
}

set<string> BinanceSystemMgr::GetInstrumentList() {
    set<string> s;
    for (auto iter = mBinanceSystemItem.begin(); iter != mBinanceSystemItem.end(); ++iter) {
        set<string> sItem = iter->second->GetInstrumentList();
        s.insert(sItem.begin(), sItem.end());
    }
    return s;
}

BinanceSystemItem* BinanceSystemMgr::GetSystemItem(int accountId) {
    BinanceSystemItem* item = nullptr;
    auto iter = mBinanceSystemItem.find(accountId);
    if (iter != mBinanceSystemItem.end()) {
        item = iter->second;
    }
    return item;
}