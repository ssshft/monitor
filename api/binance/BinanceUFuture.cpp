#include "binance/BinanceUFuture.h"
#include "Net.h"


BinanceUFuture::BinanceUFuture(AccountInfo& info) {
    // baseUrl = "https://fapi.binance.com";
    baseUrl = "https://demo-fapi.binance.com"
    accountUrl = "/fapi/v3/account";
    positionRiskUrl = "/fapi/v3/positionRisk";
    openOrderUrl = "/fapi/v1/openOrders";

    accountInfo = info;
}

BinanceUFuture::~BinanceUFuture() {
}

bool BinanceUFuture::QueryAccount(vector<binance::UFutureAsset>& vUFutureAsset, vector<binance::UFuturePosition>& vUFuturePosition, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", accountUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(baseUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceUFuture QueryAccount syncGet return false";
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceUFuture QueryAccount status: {}", status);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value& res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.HasMember("assets") && res["assets"].IsArray()) {
            const rapidjson::Value& data = res["assets"];
            for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                binance::UFutureAsset uFutureAsset;
                if (data[i].HasMember("asset")) {
                    uFutureAsset.assetType = data[i]["asset"].GetString();
                }
                if (data[i].HasMember("walletBalance")) {
                    uFutureAsset.walletBalance = std::stod(data[i]["walletBalance"].GetString());
                }
                if (data[i].HasMember("unrealizedProfit")) {
                    uFutureAsset.unrealizedProfit = std::stod(data[i]["unrealizedProfit"].GetString());
                }
                if (data[i].HasMember("marginBalance")) {
                    uFutureAsset.marginBalance = std::stod(data[i]["marginBalance"].GetString());
                }
                if (data[i].HasMember("maintMargin")) {
                    uFutureAsset.maintMargin = std::stod(data[i]["maintMargin"].GetString());
                }
                if (data[i].HasMember("initialMargin")) {
                    uFutureAsset.initialMargin = std::stod(data[i]["initialMargin"].GetString());
                }
                if (data[i].HasMember("positionInitialMargin")) {
                    uFutureAsset.positionInitialMargin = std::stod(data[i]["positionInitialMargin"].GetString());
                }
                if (data[i].HasMember("openOrderInitialMargin")) {
                    uFutureAsset.openOrderInitialMargin = std::stod(data[i]["openOrderInitialMargin"].GetString());
                }
                if (data[i].HasMember("crossWalletBalance")) {
                    uFutureAsset.crossWalletBalance = std::stod(data[i]["crossWalletBalance"].GetString());
                }
                if (data[i].HasMember("crossUnPnl")) {
                    uFutureAsset.crossUnPnl = std::stod(data[i]["crossUnPnl"].GetString());
                }
                if (data[i].HasMember("availableBalance")) {
                    uFutureAsset.availableBalance = std::stod(data[i]["availableBalance"].GetString());
                }
                if (data[i].HasMember("maxWithdrawAmount")) {
                    uFutureAsset.maxWithdrawAmount = std::stod(data[i]["maxWithdrawAmount"].GetString());
                }
                if (data[i].HasMember("marginAvailable")) {
                    uFutureAsset.marginAvailable = data[i]["marginAvailable"].GetBool();
                }
                if (data[i].HasMember("updateTime")) {
                    uFutureAsset.updateTime = std::stoll(data[i]["updateTime"].GetString());
                }

                if (fabs(uFutureAsset.walletBalance) <= 0.0000000001) {
                    continue;
                }

                LOG_INFO("QueryAccount AccountId: {}    uFutureAsset: {}", accountInfo.accountId, uFutureAsset.toString());
                vUFutureAsset.emplace_back(uFutureAsset);
            }
        }

        if (res.HasMember("positions") && res["positions"].IsArray()) {
            const rapidjson::Value& data = res["positions"];
            for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                binance::UFuturePosition uFuturePosition;
                if (data[i].HasMember("symbol")) {
                    uFuturePosition.symbol = data[i]["symbol"].GetString();
                }
                if (data[i].HasMember("initialMargin")) {
                    uFuturePosition.initialMargin = std::stod(data[i]["initialMargin"].GetString());
                }
                if (data[i].HasMember("maintMargin")) {
                    uFuturePosition.maintMargin = std::stod(data[i]["maintMargin"].GetString());
                }
                if (data[i].HasMember("unrealizedProfit")) {
                    uFuturePosition.unrealizedProfit = std::stod(data[i]["unrealizedProfit"].GetString());
                }
                if (data[i].HasMember("positionInitialMargin")) {
                    uFuturePosition.positionInitialMargin = std::stod(data[i]["positionInitialMargin"].GetString());
                }
                if (data[i].HasMember("openOrderInitialMargin")) {
                    uFuturePosition.openOrderInitialMargin = std::stod(data[i]["openOrderInitialMargin"].GetString());
                }
                if (data[i].HasMember("leverage")) {
                    uFuturePosition.leverage = std::stod(data[i]["leverage"].GetString());
                }
                if (data[i].HasMember("isolated")) {
                    uFuturePosition.isolated = data[i]["isolated"].GetBool();
                }
                if (data[i].HasMember("entryPrice")) {
                    uFuturePosition.entryPrice = std::stod(data[i]["entryPrice"].GetString());
                }
                if (data[i].HasMember("maxNotional")) {
                    uFuturePosition.maxNotional = std::stod(data[i]["maxNotional"].GetString());
                }
                if (data[i].HasMember("bidNotional")) {
                    uFuturePosition.bidNotional = std::stod(data[i]["bidNotional"].GetString());
                }
                if (data[i].HasMember("askNotional")) {
                    uFuturePosition.askNotional = std::stod(data[i]["askNotional"].GetString());
                }
                if (data[i].HasMember("positionSide")) {
                    uFuturePosition.positionSide = data[i]["positionSide"].GetString();
                }
                if (data[i].HasMember("positionAmt")) {
                    /*
                    if (cFuturePosition.positionSide == "SHORT") {
                        cFuturePosition.positionAmt = -stod(position.at("positionAmt").as_string());
                    } else {
                        cFuturePosition.positionAmt = stod(position.at("positionAmt").as_string());
                    }
                    */
                    uFuturePosition.positionAmt = std::stod(data[i]["positionAmt"].GetString());
                }
                if (data[i].HasMember("updateTime")) {
                    uFuturePosition.updateTime = std::stoll(data[i]["updateTime"].GetString());
                }
        
                if (fabs(uFuturePosition.positionAmt) <= 0.0000000001) {
                    continue;
                }

                LOG_INFO("QueryAccount AccountId: {}   uFuturePosition: {}", accountInfo.accountId, uFuturePosition.toString());
                vUFuturePosition.emplace_back(uFuturePosition);

            }
        }

        if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceUFuture QueryAccount code: {}, msg: {}", code, msg);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceUFuture QueryAccount exception: {}", e.what());
        LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool BinanceUFuture::QueryPositionRisk(std::vector<binance::PositionRisk>& vPositionRisk, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", positionRiskUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(baseUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceUFuture QueryPositionRisk syncGet return false";
            LOG_INFO("QueryPositionRisk AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceUFuture QueryPositionRisk status: {}", status);
            LOG_INFO("QueryPositionRisk AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value& res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

         if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                binance::PositionRisk positionRisk;
                if (res[i].HasMember("symbol")) {
                    positionRisk.symbol = res[i]["symbol"].GetString();
                }
                if (res[i].HasMember("liquidationPrice")) {
                    positionRisk.liquidationPrice = std::stod(res[i]["liquidationPrice"].GetString());
                }
                if (res[i].HasMember("positionSide")) {
                    positionRisk.positionSide = res[i]["positionSide"].GetString();
                }

                LOG_INFO("QueryPositionRisk AccountId: {}   uFuturePosition: {}", accountInfo.accountId, positionRisk.toString());
                vPositionRisk.emplace_back(positionRisk);
            }
        }

        if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceUFuture QueryPositionRisk code: {}, msg: {}", code, msg);
            LOG_INFO("QueryPositionRisk AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceUFuture QueryPositionRisk exception: {}", e.what());
        LOG_INFO("QueryPositionRisk AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool BinanceUFuture::QueryOpenOrder(vector<binance::FutureOpenOrder>& vOpenOrder, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", openOrderUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(baseUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceUFuture QueryOpenOrder syncGet return false";
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceUFuture QueryOpenOrder status: {}", status);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value& res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

         if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                binance::FutureOpenOrder openOrder;
                if (res[i].HasMember("symbol")) {
                    openOrder.symbol = res[i]["symbol"].GetString();
                }
                if (res[i].HasMember("id")) {
                    openOrder.orderId = std::stoll(res[i]["id"].GetString());
                }
                if (res[i].HasMember("clientOrderId")) {
                    openOrder.clientOrderId = res[i]["clientOrderId"].GetString();
                }
                if (res[i].HasMember("price")) {
                    openOrder.price = std::stod(res[i]["price"].GetString());
                }
                if (res[i].HasMember("origQty")) {
                    openOrder.origQty = std::stod(res[i]["origQty"].GetString());
                }
                if (res[i].HasMember("executedQty")) {
                    openOrder.executedQty = std::stod(res[i]["executedQty"].GetString());
                }
                if (res[i].HasMember("avgPrice")) {
                    openOrder.avgPrice = std::stod(res[i]["avgPrice"].GetString());
                }
                if (res[i].HasMember("cumQuote")) {
                    openOrder.cumQuote = std::stod(res[i]["cumQuote"].GetString());
                }
                if (res[i].HasMember("side")) {
                    openOrder.side = res[i]["side"].GetString();
                }
                if (res[i].HasMember("positionSide")) {
                    openOrder.positionSide = res[i]["positionSide"].GetString();
                }
                if (res[i].HasMember("time")) {
                    openOrder.time = std::stoll(res[i]["time"].GetString());
                }
                vOpenOrder.emplace_back(openOrder);
                LOG_INFO("BinanceUFuture AccountId: {}   QueryOpenOrder: {}", accountInfo.accountId, openOrder.toString());
            }
        }

        if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceUFuture QueryOpenOrder code: {}, msg: {}", code, msg);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceUFuture QueryOpenOrder exception: {}", e.what());
        LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}
