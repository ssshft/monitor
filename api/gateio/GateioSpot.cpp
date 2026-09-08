#include "gateio/GateioSpot.h"


GateioSpot::GateioSpot(AccountInfo& info) {
    accountUrl = "/api/v4/spot/accounts";
    openOrderUrl = "/api/v4/spot/open_orders";
    accountInfo = info;
}

GateioSpot::~GateioSpot() {
}

bool GateioSpot::QueryAccount(std::vector<gateio::SpotAsset>& vSpotAsset, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string time_str = std::to_string(crypto::getCurrentTimeSeconds());    
    std::string sign = crypto::getGateioSignatureRest("GET", accountUrl, time_str, "", "", accountInfo.secretKey);
    std::vector<std::pair<std::string, std::string>> headers = {{"KEY", accountInfo.apiKey}, {"Timestamp", time_str}, {"SIGN", sign}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), accountUrl, {}, headers, body, status)) {
            std::string errMsg = "GateioSpot QueryAccount syncGet return false";
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("GateioSpot QueryAccount status: {}", status);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                gateio::SpotAsset spotAsset;
                if (res[i].HasMember("currency")) {
                    spotAsset.currency = crypto::to_upper(res[i]["currency"].GetString());
                }
                if (res[i].HasMember("available")) {
                    spotAsset.available = std::stod(res[i]["available"].GetString());
                }
                if (res[i].HasMember("locked")) {
                    spotAsset.locked = std::stod(res[i]["locked"].GetString());
                }

                if ((spotAsset.available + spotAsset.locked) >= 0.0000000001) {
                    spotAsset.total = spotAsset.available + spotAsset.locked;
                    LOG_INFO("QueryAccount AccountId: {}   GateioSpot asset: {}", accountInfo.accountId, spotAsset.toString());
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

            std::string errMsg = fmt::format("GateioSpot QueryAccount code: {}, msg: {}", code, msg);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        } 
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("GateioSpot QueryAccount exception: {}", e.what());
        LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}


bool GateioSpot::QueryOpenOrder(std::vector<gateio::SpotOrder>& vSpotOpenOrder, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string time_str = std::to_string(crypto::getCurrentTimeSeconds());    
    std::string sign = crypto::getGateioSignatureRest("GET", openOrderUrl, time_str, "", "", accountInfo.secretKey);
    std::vector<std::pair<std::string, std::string>> headers = {{"KEY", accountInfo.apiKey}, {"Timestamp", time_str}, {"SIGN", sign}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), openOrderUrl, {}, headers, body, status)) {
            std::string errMsg = "GateioSpot QueryOpenOrder syncGet return false";
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("GateioSpot QueryOpenOrder status: {}", status);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                if (res[i].HasMember("orders") && res[i]["orders"].IsArray()) {
                    const rapidjson::Value& orders = res[i]["orders"];
                    for (rapidjson::SizeType j = 0; j < orders.Size(); ++j) {
                        gateio::SpotOrder order;
                        if (orders[j].HasMember("id")) {
                            order.id = orders[j]["id"].GetString();
                        }
                        if (orders[j].HasMember("text")) {
                            order.text = orders[j]["text"].GetString();
                        }
                        if (orders[j].HasMember("currency_pair")) {
                            order.currency_pair = orders[j]["currency_pair"].GetString();
                        }
                        if (orders[j].HasMember("status")) {
                            order.status = orders[j]["status"].GetString();
                        }
                        if (orders[j].HasMember("type")) {
                            order.type = orders[j]["type"].GetString();
                        }
                        if (orders[j].HasMember("time_in_force")) {
                            order.time_in_force = orders[j]["time_in_force"].GetString();
                        }
                        if (orders[j].HasMember("side")) {
                            order.side = orders[j]["side"].GetString();
                        }
                        if (orders[j].HasMember("amount")) {
                            order.amount = std::stod(orders[j]["amount"].GetString());
                        }
                        if (orders[j].HasMember("price")) {
                            order.price = std::stod(orders[j]["price"].GetString());
                        }
                        if (orders[j].HasMember("left")) {
                            order.left = std::stod(orders[j]["left"].GetString());
                        }
                        if (orders[j].HasMember("filled_total")) {
                            order.filled_total = std::stod(orders[j]["filled_total"].GetString());
                        }
                        if (orders[j].HasMember("update_time")) {
                            order.update_time = std::stoll(orders[j]["update_time"].GetString());
                        }

                        LOG_INFO("QueryOpenOrder AccountId: {}  spotOpenOrder: {}", accountInfo.accountId, order.toString());
                        vSpotOpenOrder.emplace_back(order);
                    }
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

            std::string errMsg = fmt::format("GateioSpot QueryOpenOrder code: {}, msg: {}", code, msg);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        } 
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("GateioSpot QueryOpenOrder exception: {}", e.what());
        LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}