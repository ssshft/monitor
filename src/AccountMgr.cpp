#include "AccountMgr.h"
#include "MdMgr.h"
#include "MonitorConfig.h"

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
    rapidjson::Document previewRes;
    previewRes.SetObject();
    auto& allocator = previewRes.GetAllocator();
    
    // 创建data对象
    rapidjson::Value previewDataV(rapidjson::kObjectType);
    
    // 创建list数组
    rapidjson::Value listArray(rapidjson::kArrayType);
    
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {       
        // 获取每个账户的preview Document
        rapidjson::Document previewDoc = iter->second->GetPreview();
        
        // 将Document的值复制到listArray中
        rapidjson::Value previewValue;
        previewValue.CopyFrom(previewDoc, allocator);
        listArray.PushBack(previewValue, allocator);
        
    }
    
    // 添加list到data对象
    previewDataV.AddMember("list", listArray, allocator);
    
    // 添加type和data到previewRes
    previewRes.AddMember("type", 4, allocator);
    previewRes.AddMember("data", previewDataV, allocator);
    
    // 序列化为字符串
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    previewRes.Accept(writer);
    
    return buffer.GetString();
}

string AccountMgr::GetPhysicalAccountStatus() {
    rapidjson::Document detailRes;
    detailRes.SetObject();
    auto& allocator = detailRes.GetAllocator();
    
    // 创建data对象
    rapidjson::Value detailDataV(rapidjson::kObjectType);
    
    // 创建list数组
    rapidjson::Value listArray(rapidjson::kArrayType);
    
    for (auto iter = mAccount.begin(); iter != mAccount.end(); ++iter) {
        string type = iter->second->GetAccountType();
        if (type == "physical") {
            // 获取每个账户的detail Document
            rapidjson::Document detailDoc = iter->second->GetDetail();
            
            // 将Document的值复制到listArray中
            rapidjson::Value detailValue;
            detailValue.CopyFrom(detailDoc, allocator);
            listArray.PushBack(detailValue, allocator);
        }
    }
    
    // 添加list到data对象
    detailDataV.AddMember("list", listArray, allocator);
    
    // 添加type和data到detailRes
    detailRes.AddMember("type", 1, allocator);
    detailRes.AddMember("data", detailDataV, allocator);
    
    // 序列化为字符串
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    detailRes.Accept(writer);
    
    return buffer.GetString();
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
