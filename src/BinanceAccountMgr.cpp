#include "BinanceAccountMgr.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"
#include "MarketInfo.h"

BinanceAccountMgr::BinanceAccountMgr() {
    unordered_map<int, AccountInfo>& mAccountInfo = MonitorConfig::GetInstance().GetAccountInfo();
    for (auto iter = mAccountInfo.begin(); iter != mAccountInfo.end(); ++iter) {
        string exchangeStr = ExchangeTypeEnum2StrMap[iter->second.exchangeType];
        mAccount[iter->first] = new BinanceAccountItem(iter->first, iter->second.accountName, "physical", exchangeStr, iter->second.hedge);
    }

    vector<int>& vStrategyAccount = MonitorConfig::GetInstance().GetStrategyAccount();
    for (size_t i = 0; i < vStrategyAccount.size(); ++i) {
        int accountId = vStrategyAccount[i];
        mAccount[accountId] = new BinanceAccountItem(accountId, "", "strategy", "", 1);
    }

    openInterestAlarm = MonitorConfig::GetInstance().GetOpenInterestAlarm();
}

BinanceAccountMgr::~BinanceAccountMgr() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        delete iter->second;
    }
    mAccount.clear();
}

BinanceAccountItem* BinanceAccountMgr::GetAccountItem(int accountId) {
    BinanceAccountItem* item = nullptr;
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

BinanceAccountMgr& BinanceAccountMgr::GetInstance() {
	static BinanceAccountMgr binanceAccountMgr;
	return binanceAccountMgr;
}

/*
void BinanceAccountMgr::OnSubMessage(const web::json::value& content) {
    int cmdType = content.at("cmd_type").as_integer();
    if (cmdType == ASSET_AND_POSITION_UPDATE_REPORT) {  // 资产持仓更新
        int type = content.at("type").as_integer();
        if (type == ASSET_ALL) {                        // 全量
            int customerId = content.at("customer_id").as_integer();
            auto iter = mAccount.find(customerId);
            if (iter != mAccount.end()) {
                iter->second->OnSubMessage(content);
            }
        }
    }
}
*/

void BinanceAccountMgr::UpdateBySystem() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
     
    }
}

void BinanceAccountMgr::UpdateByAdapter() {
    std::lock_guard<std::mutex> lk(mut);
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->UpdateByAdapter();
    }
}

void BinanceAccountMgr::ClearZero() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->ClearZero();
    }
}

void BinanceAccountMgr::Clear() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->Clear();
    }
}

void BinanceAccountMgr::CalculateTotalAsset() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->CalculateTotalAsset();
    }
}

void BinanceAccountMgr::CalculateExposure() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->CalculateExposure();
    }
}

void BinanceAccountMgr::CalculateRiskInfo() {
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        iter->second->CalculateRiskInfo();
    }
}

string BinanceAccountMgr::GetPhysicalOverView() {
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

string BinanceAccountMgr::GetPhysicalAccountStatus() {
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

string BinanceAccountMgr::GetStrategyAccountStatus() {
    vector<web::json::value> detailV;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        string type = iter->second->GetAccountType();
        if (type == "strategy") {
            detailV.push_back(iter->second->GetDetail());
        }
    }
    web::json::value detailDataV;
    detailDataV["list"] = web::json::value::array(detailV);
    web::json::value detailRes;
    detailRes["type"] = web::json::value::number(2);
    detailRes["data"] = detailDataV;
    return detailRes.serialize();
}

string BinanceAccountMgr::GetMarketRiskStatus() {
    mMarketInfo.clear();

    web::json::value marketInfoV;
    unordered_map<string, SymbolMarketInfo>& mSymbolMarketInfo = MarketInfo::GetInstance().GetSymbolMarketInfo();
    for (auto iter = mTotalAsset.begin(); iter != mTotalAsset.end(); iter++) {
        if (fabs(iter->second.netAmountD) <= 0.0000000001) {
            continue;
        }

        LOG_INFO("mTotalAsset  key:%s  symbol:%s  volume:%f", iter->first.c_str(), iter->second.asset.c_str(), iter->second.netAmountD);

        double volume = 0;
        double openInterest = 0;
        double liquid = 0;
        double concentration = 0;
        auto it = mSymbolMarketInfo.find(iter->first);
        if (it != mSymbolMarketInfo.end()) {
            volume = it->second.volume;
            openInterest = it->second.openInterest;
            if (volume > 0) {
                liquid = fabs(iter->second.netAmountD / volume);
            }
        }

        igmonitor::MarketInfo mi;
        mi.instrumentKey = iter->first;
        mi.amount = iter->second.netAmountD;
        mi.twentyHourAmount = volume;
        mi.openInterest = openInterest;
        mi.liquid = liquid;
        mi.concentration = concentration;
        mMarketInfo[mi.instrumentKey] = mi;
    }

    for (auto iter = mTotalPosition.begin(); iter != mTotalPosition.end(); iter++) {
        if (fabs(iter->second.netPositionD) <= 0.0000000001) {
            continue;
        }

        LOG_INFO("mTotalPosition  key:%s  symbol:%s  volume:%f", iter->first.c_str(), iter->second.symbol.c_str(), iter->second.netPositionD);

        double volume = 0;
        double openInterest = 0;
        double liquid = 0;
        double concentration = 0;
        auto it = mSymbolMarketInfo.find(iter->first);
        if (it != mSymbolMarketInfo.end()) {
            volume = it->second.volume;
            openInterest = it->second.openInterest;
            if (volume > 0) {
                liquid = fabs(iter->second.netPositionD / volume);
            }
            if (openInterest > 0) {
                concentration = fabs(iter->second.netPositionD / openInterest);
            }
        }

        igmonitor::MarketInfo mi;
        mi.instrumentKey = iter->first;
        mi.amount = iter->second.netPositionD;
        mi.twentyHourAmount = volume;
        mi.openInterest = openInterest;
        mi.liquid = liquid;
        mi.concentration = concentration;
        mMarketInfo[mi.instrumentKey] = mi;
    }

    for (auto iter = mMarketInfo.begin(); iter != mMarketInfo.end(); ++iter) {
        web::json::value marketInfo;
		marketInfo["instrument_key"] = web::json::value::string(iter->first);
		marketInfo["amount"] = web::json::value::number(iter->second.amount);
		marketInfo["24hAmount"] = web::json::value::number(iter->second.twentyHourAmount);
		marketInfo["openInterest"] = web::json::value::number(iter->second.openInterest);
        marketInfo["liquid"] = web::json::value::number(iter->second.liquid);
		marketInfo["concentration"] = web::json::value::number(iter->second.concentration);
        marketInfoV[iter->first] = marketInfo; 
    }

	web::json::value listV;
	listV["marketInfo"] = marketInfoV;
    listV["fundingRate"] = BinanceMdMgr::GetInstance().GetFundingRateValue();

	web::json::value dataV;
	dataV["list"] = listV;

	web::json::value res;
	res["type"] = web::json::value::number(4);
	res["data"] = dataV;

    return res.serialize();
}

void BinanceAccountMgr::CalculateAccount() {
    Clear();
    UpdateBySystem();
    UpdateByAdapter();
    CalculateTotalAsset();
    CalculateExposure();
    CalculateRiskInfo();
    StatisticAccount();
}

void BinanceAccountMgr::StatisticAccount() {
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

set<string> BinanceAccountMgr::GetInstrumentList() {
    set<string> s;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        set<string> sItem = iter->second->GetInstrumentList();
        s.insert(sItem.begin(), sItem.end());
    }
    return s;
}

vector<MsgCard> BinanceAccountMgr::GetAlarmMsg() {
    vector<MsgCard> v;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        vector<MsgCard> vAlarmMsg = iter->second->GetAlarmMsg();
        v.insert(v.end(), vAlarmMsg.begin(), vAlarmMsg.end());
    }
    return v;
}

vector<MsgCard> BinanceAccountMgr::GetFundingRateAlarmMsg() {
    vector<MsgCard> v;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        vector<MsgCard> vAlarmMsg = iter->second->GetFundingRateAlarmMsg();
        v.insert(v.end(), vAlarmMsg.begin(), vAlarmMsg.end());
    }
    return v;
}

vector<MsgCard> BinanceAccountMgr::GetPositionLiquidationPriceAlarmMsg() {
    vector<MsgCard> v;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        vector<MsgCard> vAlarmMsg = iter->second->GetPositionLiquidationPriceAlarmMsg();
        v.insert(v.end(), vAlarmMsg.begin(), vAlarmMsg.end());
    }
    return v;
}

vector<igmonitor::RiskInfo> BinanceAccountMgr::GetRiskInfo() {
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

vector<MsgCard> BinanceAccountMgr::GetMarketStatusAlarmMsg() {
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

vector<MsgCard> BinanceAccountMgr::GetOrderAlarmMsg() {
    vector<MsgCard> v;
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        vector<MsgCard> vAlarmMsg = iter->second->GetOrderAlarmMsg();
        v.insert(v.end(), vAlarmMsg.begin(), vAlarmMsg.end());
    }
    return v;
}