#include "gateio/GateioUnified.h"


GateioUnified::GateioUnified(AccountInfo& info) {
    accountUrl = "/api/v4/unified/accounts";
    accountInfo = info;
}

GateioUnified::~GateioUnified() {
}

bool GateioUnified::QueryAccount(gateio::UnifyTotalAccount& unifyTotalAccount, std::vector<gateio::SpotAsset>& vSpotAsset, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string time_str = std::to_string(crypto::getCurrentTimeSeconds());    
    std::string sign = crypto::getGateioSignatureRest("GET", accountUrl, time_str, "", "", accountInfo.secretKey);
    std::vector<std::pair<std::string, std::string>> headers = {{"KEY", accountInfo.apiKey}, {"Timestamp", time_str}, {"SIGN", sign}};

    try {
        if (!Net::Instance().syncGet(crypto::host_of(accountInfo.restUrl), accountUrl, {}, headers, body, status)) {
            std::string errMsg = "GateioUnified QueryAccount syncGet return false";
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("GateioUnified QueryAccount status: {}", status);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.HasMember("total")) {
            unifyTotalAccount.total = std::stod(res["total"].GetString());
        }
        if (res.HasMember("borrowed")) {
            unifyTotalAccount.borrowed = std::stod(res["borrowed"].GetString());
        }
        if (res.HasMember("total_initial_margin")) {
            unifyTotalAccount.totalInitialMargin = std::stod(res["total_initial_margin"].GetString());
        }
        if (res.HasMember("total_margin_balance")) {
            unifyTotalAccount.totalMarginBalance = std::stod(res["total_margin_balance"].GetString());
        }
        if (res.HasMember("total_maintenance_margin")) {
            unifyTotalAccount.totalMaintenanceMargin = std::stod(res["total_maintenance_margin"].GetString());
        }
        if (res.HasMember("total_initial_margin_rate")) {
            unifyTotalAccount.totalInitialMarginRate = std::stod(res["total_initial_margin_rate"].GetString());
        }
        if (res.HasMember("total_maintenance_margin_rate")) {
            unifyTotalAccount.totalMaintenanceMarginRate = std::stod(res["total_maintenance_margin_rate"].GetString());
        }
        if (res.HasMember("total_available_margin")) {
            unifyTotalAccount.totalAvailableMargin = std::stod(res["total_available_margin"].GetString());
        }
        if (res.HasMember("unified_account_total")) {
            unifyTotalAccount.unifiedAccountTotal = std::stod(res["unified_account_total"].GetString());
        }
        if (res.HasMember("unified_account_total_liab")) {
            unifyTotalAccount.unifiedAccountTotalLiab = std::stod(res["unified_account_total_liab"].GetString());
        }
        if (res.HasMember("unified_account_total_equity")) {
            unifyTotalAccount.unifiedAccountTotalEquity = std::stod(res["unified_account_total_equity"].GetString());
        }
        if (res.HasMember("leverage")) {
            unifyTotalAccount.leverage = std::stod(res["leverage"].GetString());
        }

        if (res.HasMember("balances")) {
            const rapidjson::Value& balances = res["balances"];
            
            // 遍历 balances 对象的所有成员
            for (rapidjson::Value::ConstMemberIterator it = balances.MemberBegin();  it != balances.MemberEnd(); ++it) {
                gateio::SpotAsset spotAsset;
               
                spotAsset.currency = it->name.GetString();  // 获取币种名称（如 "ETH", "POINT"）
                const rapidjson::Value& detail = it->value;
                if (detail.HasMember("available")) {
                    spotAsset.available = std::stod(detail["available"].GetString());
                }
                if (detail.HasMember("freeze") && detail["freeze"].IsString()) {
                     spotAsset.locked = std::stod(detail["freeze"].GetString());
                }

                if ((spotAsset.available + spotAsset.locked) >= 0.0000000001) {
                    spotAsset.total = spotAsset.available + spotAsset.locked;
                    LOG_INFO("QueryAccount AccountId: {}   GateioUnified asset: {}", accountInfo.accountId, spotAsset.toString());
                    vSpotAsset.emplace_back(spotAsset);
                }
            }
        }

        if (res.has_field("label")) {
            query = false;
            std::string label = res["label"].GetString();
            std::string msg = "";
            if (label.size() > 0) {
                if (res.HasMember("message")) {
                    msg = res["message"].GetString();
                } else {
                    msg = label;
                }
            }

            std::string errMsg = fmt::format("GateioUnified QueryAccount code: {}, msg: {}", code, msg);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        } 
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("GateioUnified QueryAccount exception: {}", e.what());
        LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}
