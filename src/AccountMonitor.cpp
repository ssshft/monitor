#include "AccountMonitor.h"
#include "MonitorConfig.h"
#include "BinanceAccountMgr.h"
#include "BinanceSystemMgr.h"
#include "ProductMgr.h"
#include "DataQueue.h"


AccountMonitor::AccountMonitor() {
    vector<AccountMonitorInfo>& v = MonitorConfig::GetInstance().GetAccountMonitorInfo();
    for (size_t i = 0; i < v.size(); ++i) {
        AccountMonitorItem* item = new AccountMonitorItem(v[i]);
        vAccountMonitorItem.push_back(item);
    }

    dealSubFlag = true;
    dealSubData = new thread(&AccountMonitor::DealSubData, this);
}

AccountMonitor::~AccountMonitor() {
    for (size_t i = 0; i < vAccountMonitorItem.size(); ++i) {
        if (vAccountMonitorItem[i]) {
            delete vAccountMonitorItem[i];
        }
    }
    vAccountMonitorItem.clear();
    dealSubFlag = false;
    if (dealSubData) {
        delete dealSubData;
        dealSubData = nullptr;
    }
    mAccountStatus.clear();
}

AccountMonitor& AccountMonitor::GetInstance() {
	static AccountMonitor accountMonitor;
	return accountMonitor;
}

void AccountMonitor::DealSubData() {
    while (dealSubFlag) {
		SubData data;
		DataQueue::GetInstance().PopTradeData(data);
	    try {
            const web::json::value& content = web::json::value::parse(data.content.c_str());
            int exchangeType = content.at("exchange_id").as_integer();
            if (exchangeType == ET_BINANCE || exchangeType == ET_COINBASE || exchangeType == ET_GATEIO) {
                BinanceSystemMgr::GetInstance().OnSubMessage(content);
            }
        } catch(exception& e) {
    	    LOG_DEBUG("Recv Error Message: '%s'", e.what());
	    }
    }
}

unordered_map<string, string>& AccountMonitor::GetCurrentStatus() {
    LOG_INFO("CalculateAccount before");
    BinanceAccountMgr::GetInstance().CalculateAccount();
    ProductMgr::GetInstance().CalculateAccount();
    LOG_INFO("CalculateAccount after");
    // 不pub信息
    /*
    mAccountStatus["overview"] = BinanceAccountMgr::GetInstance().GetPhysicalOverView();
    mAccountStatus["physical"] = BinanceAccountMgr::GetInstance().GetPhysicalAccountStatus();
    mAccountStatus["strategy"] = BinanceAccountMgr::GetInstance().GetStrategyAccountStatus();
    mAccountStatus["mdstatus"] = BinanceAccountMgr::GetInstance().GetMarketRiskStatus();
    */
    return mAccountStatus;
}

vector<MsgCard> AccountMonitor::GetAlarmMsg() {
    vector<MsgCard> v;
    vector<MsgCard> vAccountMgr = BinanceAccountMgr::GetInstance().GetAlarmMsg();
    v.insert(v.end(), vAccountMgr.begin(), vAccountMgr.end());

    vector<MsgCard> vAccountMgrPrice = BinanceAccountMgr::GetInstance().GetPositionLiquidationPriceAlarmMsg();
    v.insert(v.end(), vAccountMgrPrice.begin(), vAccountMgrPrice.end());

    vector<MsgCard> vProductMgr = ProductMgr::GetInstance().GetAlarmMsg();
    v.insert(v.end(), vProductMgr.begin(), vProductMgr.end());
    return v;
}

vector<MsgCard> AccountMonitor::GetFundingRateAlarmMsg() {
    vector<MsgCard> v;
    vector<MsgCard> vAccountMgr = BinanceAccountMgr::GetInstance().GetFundingRateAlarmMsg();
    v.insert(v.end(), vAccountMgr.begin(), vAccountMgr.end());
    vector<MsgCard> vProductMgr = ProductMgr::GetInstance().GetFundingRateAlarmMsg();
    v.insert(v.end(), vProductMgr.begin(), vProductMgr.end());
    return v;
}
