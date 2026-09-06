#include "AccountMgr.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"
#include "MarketInfo.h"

AccountMgr::AccountMgr() {

}

AccountMgr::~AccountMgr() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        delete iter->second;
    }
    mAccount.clear();
}

void AccountMgr::Init(sm::SecurityManager* s) {
    unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        string exchangeStr = ExchangeTypeEnum2StrMap[iter->second.exchangeType];
        mAccount[iter->first] = new AccountItem(iter->first, iter->second.accountName, "physical", exchangeStr, iter->second.hedge, s);
    }
}

AccountItem* AccountMgr::GetAccountItem(int accountId) {
    AccountItem* item = nullptr;
    auto iter = mAccount.find(accountId);
    if (iter != mAccount.end()) {
        item = iter->second;
    } else {
        string idStr = to_string(accountId);
        if (idStr.size() >= 4) {
            string accId = idStr.substr(0, 4);
            for (auto it = mAccount.begin(); it != mAccount.end(); ++it) {
                string accountIdStr = to_string(it->first);
                if (accountIdStr.size() >= 4) {
                    string accIdStr = accountIdStr.substr(0, 4);
                    if (accId == accIdStr) {
                        item = it->second;
                    }
                }
            }
        }
    }
    return item;
}

AccountMgr& AccountMgr::GetInstance() {
	static AccountMgr binanceAccountMgr;
	return binanceAccountMgr;
}

void AccountMgr::UpdateByAdapter() {
    std::lock_guard<std::mutex> lk(mut);
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->UpdateByAdapter();
    }
}

void AccountMgr::ClearZero() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->ClearZero();
    }
}

void AccountMgr::Clear() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->Clear();
    }
}

void AccountMgr::CalculateTotalAsset() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->CalculateTotalAsset();
    }
}

void AccountMgr::CalculateExposure() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->CalculateExposure();
    }
}

void AccountMgr::CalculateRiskInfo() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->CalculateRiskInfo();
    }
}

string AccountMgr::GetPhysicalOverView() {
    vector<web::json::value> previewV;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        string type = iter->second->GetAccountType();
        if (type == "physical") {
            previewV.push_back(iter->second->GetPreview());
        }
    }
	web::json::value previewDataV;
	previewDataV["list"] = web::json::value::array(previewV);
	web::json::value previewRes;
	previewRes["type"] = web::json::value::number(4);
	previewRes["data"] = previewDataV;
    return previewRes.serialize();
}

string AccountMgr::GetPhysicalAccountStatus() {
    vector<web::json::value> detailV;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        string type = iter->second->GetAccountType();
        if (type == "physical") {
            detailV.push_back(iter->second->GetDetail());
        }
    }
    web::json::value detailDataV;
    detailDataV["list"] = web::json::value::array(detailV);
    web::json::value detailRes;
    detailRes["type"] = web::json::value::number(1);
    detailRes["data"] = detailDataV;
    return detailRes.serialize();
}

void AccountMgr::CalculateAccount() {
    Clear();
    UpdateByAdapter();
    CalculateTotalAsset();
    CalculateExposure();
    CalculateRiskInfo();
    StatisticAccount();
}

void AccountMgr::StatisticAccount() {
    mTotalAsset.clear();
    mTotalPosition.clear();
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        if (iter->second->GetExchangeStr() == "BINANCE") {
            unordered_map<string, igmonitor::Asset>& totalAsset = iter->second->GetTotalAsset();
            for (auto it = totalAsset.begin(); it != totalAsset.end(); ++it) {
                string key = "BINANCE|" + it->first + "-USDT|SPOT";
                mTotalAsset[key].netAmountD += it->second.netAmountD;
            }

            unordered_map<string, igmonitor::Position>& uFuturePosition = iter->second->GetUFuturePosition();
            for (auto it = uFuturePosition.begin(); it != uFuturePosition.end(); ++it) {
                mTotalPosition[it->second.key].netPositionD += it->second.netPositionD;
            }

            unordered_map<string, igmonitor::Position>& cFuturePosition = iter->second->GetCFuturePosition();
            for (auto it = cFuturePosition.begin(); it != cFuturePosition.end(); ++it) {
                mTotalPosition[it->second.key].netPositionD += it->second.netPositionD;
            }
        }
    }
}

set<string> AccountMgr::GetInstrumentList() {
    set<string> s;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        set<string> sItem = iter->second->GetInstrumentList();
        s.insert(sItem.begin(), sItem.end());
    }
    return s;
}

vector<MsgCard> AccountMgr::GetAlarmMsg() {
    vector<MsgCard> v;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        vector<MsgCard> vAlarmMsg = iter->second->GetAlarmMsg();
        v.insert(v.end(), vAlarmMsg.begin(), vAlarmMsg.end());
    }
    return v;
}

vector<MsgCard> AccountMgr::GetFundingRateAlarmMsg() {
    vector<MsgCard> v;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        vector<MsgCard> vAlarmMsg = iter->second->GetFundingRateAlarmMsg();
        v.insert(v.end(), vAlarmMsg.begin(), vAlarmMsg.end());
    }
    return v;
}

vector<MsgCard> AccountMgr::GetPositionLiquidationPriceAlarmMsg() {
    vector<MsgCard> v;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        vector<MsgCard> vAlarmMsg = iter->second->GetPositionLiquidationPriceAlarmMsg();
        v.insert(v.end(), vAlarmMsg.begin(), vAlarmMsg.end());
    }
    return v;
}

vector<igmonitor::RiskInfo> AccountMgr::GetRiskInfo() {
    vector<igmonitor::RiskInfo> v;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        bool query = iter->second->GetAdapterQueryStatus();
        if (query) {
            igmonitor::RiskInfo& riskInfo = iter->second->GetRiskInfo();
            v.emplace_back(riskInfo);
        }
    }
    return v;
}

vector<MsgCard> AccountMgr::GetMarketStatusAlarmMsg() {
    int64_t currentTime = gettickcount();
    vector<MsgCard> v;
    if (openInterestAlarm > 0) {
        for (auto iter = mMarketInfo.begin(); iter != mMarketInfo.end(); ++iter) {
            if (fabs(iter->second.concentration) >= openInterestAlarm) {   
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "Symbol: " << iter->first << " 集中度超过阈值:" << openInterestAlarm << " 当前集中度=" << iter->second.concentration;
                
                MsgCard msgCard;
                msgCard.templateId = 2;
                msgCard.title = "MarketInfo";
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }
    }

    return v;
}

vector<MsgCard> AccountMgr::GetOrderAlarmMsg() {
    vector<MsgCard> v;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        vector<MsgCard> vAlarmMsg = iter->second->GetOrderAlarmMsg();
        v.insert(v.end(), vAlarmMsg.begin(), vAlarmMsg.end());
    }
    return v;
}