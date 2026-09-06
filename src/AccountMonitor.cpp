#include "AccountMonitor.h"
#include "MonitorConfig.h"
#include "AccountMgr.h"
#include "ProductMgr.h"
#include "DataQueue.h"


AccountMonitor::AccountMonitor() {

}

AccountMonitor::~AccountMonitor() {
    mAccountStatus.clear();
}

AccountMonitor& AccountMonitor::GetInstance() {
	static AccountMonitor accountMonitor;
	return accountMonitor;
}

unordered_map<string, string>& AccountMonitor::GetCurrentStatus() {
    AccountMgr::GetInstance().CalculateAccount();
    ProductMgr::GetInstance().CalculateAccount();
    
    mAccountStatus["overview"] = AccountMgr::GetInstance().GetPhysicalOverView();
    mAccountStatus["physical"] = AccountMgr::GetInstance().GetPhysicalAccountStatus();

    return mAccountStatus;
}

vector<MsgCard> AccountMonitor::GetAlarmMsg() {
    vector<MsgCard> v;
    vector<MsgCard> vAccountMgr = AccountMgr::GetInstance().GetAlarmMsg();
    v.insert(v.end(), vAccountMgr.begin(), vAccountMgr.end());

    vector<MsgCard> vAccountMgrPrice = AccountMgr::GetInstance().GetPositionLiquidationPriceAlarmMsg();
    v.insert(v.end(), vAccountMgrPrice.begin(), vAccountMgrPrice.end());

    vector<MsgCard> vAccountMgrOrder = AccountMgr::GetInstance().GetOrderAlarmMsg();
    v.insert(v.end(), vAccountMgrOrder.begin(), vAccountMgrOrder.end());

    vector<MsgCard> vProductMgr = ProductMgr::GetInstance().GetAlarmMsg();
    v.insert(v.end(), vProductMgr.begin(), vProductMgr.end());
    return v;
}

vector<MsgCard> AccountMonitor::GetFundingRateAlarmMsg() {
    vector<MsgCard> v;
    vector<MsgCard> vAccountMgr = AccountMgr::GetInstance().GetFundingRateAlarmMsg();
    v.insert(v.end(), vAccountMgr.begin(), vAccountMgr.end());
    vector<MsgCard> vProductMgr = ProductMgr::GetInstance().GetFundingRateAlarmMsg();
    v.insert(v.end(), vProductMgr.begin(), vProductMgr.end());
    return v;
}
