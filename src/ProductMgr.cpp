#include "ProductMgr.h"
#include "MonitorConfig.h"

ProductMgr::ProductMgr() {
    unordered_map<string, ProductInfo>& mProductInfo = MonitorConfig::GetInstance().GetProductInfo();
    for (auto iter = mProductInfo.begin(); iter != mProductInfo.end(); ++iter) {
        mProduct[iter->first] = new ProductItem(iter->second.productName, iter->second.vAccountId);
    }
}

ProductMgr::~ProductMgr() {
    for (auto iter = mProduct.begin(); iter != mProduct.end(); ++iter) {
        delete iter->second;
        iter->second = nullptr;
    }
    mProduct.clear();
}

ProductMgr& ProductMgr::GetInstance() {
	static ProductMgr productMgr;
	return productMgr;
}

void ProductMgr::CalculateRiskInfo() {
    for (auto iter = mProduct.begin(); iter != mProduct.end(); ++iter) {
        iter->second->CalculateRiskInfo();
    }
}

void ProductMgr::CalculateAccount() {
    CalculateRiskInfo();
}

vector<MsgCard> ProductMgr::GetAlarmMsg() {
    vector<MsgCard> v;
    for (auto iter = mProduct.begin(); iter != mProduct.end(); ++iter) {
        vector<MsgCard> vAlarmMsg = iter->second->GetAlarmMsg();
        v.insert(v.end(), vAlarmMsg.begin(), vAlarmMsg.end());
    }
    return v;
}

vector<MsgCard> ProductMgr::GetFundingRateAlarmMsg() {
    vector<MsgCard> v;
    for (auto iter = mProduct.begin(); iter != mProduct.end(); ++iter) {
        vector<MsgCard> vAlarmMsg = iter->second->GetFundingRateAlarmMsg();
        v.insert(v.end(), vAlarmMsg.begin(), vAlarmMsg.end());
    }
    return v;
}

void ProductMgr::UpdateAccountInfo() {
    for (auto iter = mProduct.begin(); iter != mProduct.end(); ++iter) {
    	iter->second->UpdateAccountInfo();
    }
    sleep(10);
}

vector<igmonitor::RiskInfo> ProductMgr::GetRiskInfo() {
    vector<igmonitor::RiskInfo> v;
    for (auto iter = mProduct.begin(); iter != mProduct.end(); ++iter) {
        bool query = iter->second->GetAdapterQueryStatus();
        if (query) {
            igmonitor::RiskInfo& riskInfo = iter->second->GetRiskInfo();
            v.emplace_back(riskInfo);
        }
    }
    return v;
}
