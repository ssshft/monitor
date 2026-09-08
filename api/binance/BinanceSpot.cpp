#include "binance/BinanceSpot.h"
#include "Net.h"


BinanceSpot::BinanceSpot(AccountInfo& info) {
    accountUrl = "/api/v3/account";
    openOrderUrl = "/api/v3/openOrders";
    accountInfo = info;
}

BinanceSpot::~BinanceSpot() {
}

bool BinanceSpot::QueryAccount(std::vector<binance::SpotAsset>& vSpotAsset, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", accountUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceSpot QueryAccount syncGet return false";
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceSpot QueryAccount status: {}", status);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.HasMember("balances") && res["balances"].IsArray()) {
            const rapidjson::Value& data = res["balances"];
            for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                binance::SpotAsset spotAsset;
                if (data[i].HasMember("asset")) {
                    spotAsset.assetType = data[i]["asset"].GetString();
                }

                if (data[i].HasMember("free")) {
                    spotAsset.free = std::stod(data[i]["free"].GetString(););
                }

                if (data[i].HasMember("locked")) {
                    spotAsset.locked = std::stod(data[i]["locked"].GetString(););
                }
            
                if (fabs(spotAsset.free + spotAsset.locked) <= 0.0000000001) {
                    continue;
                }

                LOG_INFO("QueryAccount AccountId: {}  spotAsset: {}", accountInfo.accountId, spotAsset.toString());
                vSpotAsset.emplace_back(spotAsset);
            }
        }
        else if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceSpot QueryAccount code: {}, msg: {}", code, msg);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceSpot QueryAccount exception: {}", e.what());
        LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool BinanceSpot::QueryOpenOrder(std::vector<binance::SpotOpenOrder> vSpotOpenOrder, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", openOrderUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceSpot QueryAccount syncGet return false";
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceSpot QueryAccount status: {}", status);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                binance::SpotOpenOrder spotOpenOrder;
                if (res[i].HasMember("symbol")) {
                    spotOpenOrder.symbol = res[i]["symbol"].GetString();
                }

                if (res[i].HasMember("orderId")) {
                    spotOpenOrder.orderId = std::stoll(res[i]["orderId"].GetString());
                }
            
                if (res[i].HasMember("clientOrderId")) {
                    spotOpenOrder.clientOrderId = res[i]["clientOrderId"].GetString();
                }                 

                if (res[i].HasMember("price")) {
                    spotOpenOrder.price = std::stod(res[i]["price"].GetString());
                }

                if (res[i].HasMember("origQty")) {
                    spotOpenOrder.origQty = std::stod(res[i]["origQty"].GetString());
                }

                if (res[i].HasMember("executedQty")) {
                    spotOpenOrder.executedQty = std::stod(res[i]["executedQty"].GetString());
                }

                if (res[i].HasMember("cummulativeQuoteQty")) {
                    spotOpenOrder.cumQuote = std::stod(res[i]["cummulativeQuoteQty"].GetString());
                }

                if (res[i].HasMember("side")) {
                    spotOpenOrder.side = res[i]["side"].GetString();
                }

                if (res[i].HasMember("time")) {
                    spotOpenOrder.time = std::stoll(res[i]["time"].GetString());
                }

                LOG_INFO("QueryOpenOrder AccountId: {}  spotOpenOrder: {}", accountInfo.accountId, spotOpenOrder.toString());
                vSpotOpenOrder.emplace_back(spotOpenOrder);
            }
        }
        else if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceSpot QueryOpenOrder code: {}, msg: {}", code, msg);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceSpot QueryOpenOrder exception: {}", e.what());
        LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}
