#pragma once

#include <string>
#include <unordered_map>
#include "ProductItem.h"

using namespace std;


class ProductMgr {
public:
	static ProductMgr& GetInstance();
	~ProductMgr();
    void CalculateRiskInfo();
    void CalculateAccount();
    vector<MsgCard> GetAlarmMsg();
    vector<MsgCard> GetFundingRateAlarmMsg();
    void UpdateAccountInfo();
    vector<igmonitor::RiskInfo> GetRiskInfo();

private:
	ProductMgr();
    unordered_map<string, ProductItem*> mProduct;
};
