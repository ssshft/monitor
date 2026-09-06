#pragma once

#include <string>
#include <unordered_map>
#include "ProductItem.h"


class ProductMgr {
public:
	static ProductMgr& GetInstance();
	~ProductMgr();
    void CalculateRiskInfo();
    void CalculateAccount();
    std::vector<MsgCard> GetAlarmMsg();
    std::vector<MsgCard> GetFundingRateAlarmMsg();
    void UpdateAccountInfo();
    std::vector<igmonitor::RiskInfo> GetRiskInfo();

private:
	ProductMgr();
    std::unordered_map<string, ProductItem*> mProduct;
};
