#include "binance/BinanceCFuture.h"


BinanceCFuture::BinanceCFuture(AccountInfo& info) {
    baseUrl = "https://dapi.binance.com";
    accountUrl = "/dapi/v1/account";
    positionRiskUrl = "/dapi/v1/positionRisk";
    openOrderUrl = "/dapi/v1/openOrders";
    accountInfo = info;
}

BinanceCFuture::~BinanceCFuture() {
}

bool BinanceCFuture::QueryAccount(std::vector<binance::CFutureAsset>& vCFutureAsset, std::vector<binance::CFuturePosition>& vCFuturePosition, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", accountUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(baseUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceCFuture QueryAccount syncGet return false";
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceCFuture QueryAccount status: {}", status);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value& res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.HasMember("assets") && res["assets"].IsArray()) {
            const rapidjson::Value& data = res["assets"];
            for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                binance::CFutureAsset cFutureAsset;
                if (data[i].HasMember("asset")) {
                    cFutureAsset.assetType = data[i]["asset"].GetString();
                }
                if (data[i].HasMember("walletBalance")) {
                    cFutureAsset.walletBalance = std::stod(data[i]["walletBalance"].GetString());
                }
                if (data[i].HasMember("unrealizedProfit")) {
                    cFutureAsset.unrealizedProfit = std::stod(data[i]["unrealizedProfit"].GetString());
                }
                if (data[i].HasMember("marginBalance")) {
                    cFutureAsset.marginBalance = std::stod(data[i]["marginBalance"].GetString());
                }
                if (data[i].HasMember("maintMargin")) {
                    cFutureAsset.maintMargin = std::stod(data[i]["maintMargin"].GetString());
                }
                if (data[i].HasMember("initialMargin")) {
                    cFutureAsset.initialMargin = std::stod(data[i]["initialMargin"].GetString());
                }
                if (data[i].HasMember("positionInitialMargin")) {
                    cFutureAsset.positionInitialMargin = std::stod(data[i]["positionInitialMargin"].GetString());
                }
                if (data[i].HasMember("openOrderInitialMargin")) {
                    cFutureAsset.openOrderInitialMargin = std::stod(data[i]["openOrderInitialMargin"].GetString());
                }
                if (data[i].HasMember("crossWalletBalance")) {
                    cFutureAsset.crossWalletBalance = std::stod(data[i]["crossWalletBalance"].GetString());
                }
                if (data[i].HasMember("crossUnPnl")) {
                    cFutureAsset.crossUnPnl = std::stod(data[i]["crossUnPnl"].GetString());
                }
                if (data[i].HasMember("availableBalance")) {
                    cFutureAsset.availableBalance = std::stod(data[i]["availableBalance"].GetString());
                }
                if (data[i].HasMember("maxWithdrawAmount")) {
                    cFutureAsset.maxWithdrawAmount = std::stod(data[i]["maxWithdrawAmount"].GetString());
                }
            
                if (fabs(cFutureAsset.walletBalance) <= 0.0000000001) {
                    continue;
                }

                LOG_INFO("QueryAccount AccountId: {}    cFutureAsset: {}", accountInfo.accountId, cFutureAsset.toString());
                vCFutureAsset.emplace_back(cFutureAsset);
            }
        }

        if (res.HasMember("positions") && res["positions"].IsArray()) {
            const rapidjson::Value& data = res["positions"];
            for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                binance::CFuturePosition cFuturePosition;
                if (data[i].HasMember("symbol")) {
                    cFuturePosition.symbol = data[i]["symbol"].GetString();
                }
                if (data[i].HasMember("initialMargin")) {
                    cFuturePosition.initialMargin = std::stod(data[i]["initialMargin"].GetString());
                }
                if (data[i].HasMember("maintMargin")) {
                    cFuturePosition.maintMargin = std::stod(data[i]["maintMargin"].GetString());
                }
                if (data[i].HasMember("unrealizedProfit")) {
                    cFuturePosition.unrealizedProfit = std::stod(data[i]["unrealizedProfit"].GetString());
                }
                if (data[i].HasMember("positionInitialMargin")) {
                    cFuturePosition.positionInitialMargin = std::stod(data[i]["positionInitialMargin"].GetString());
                }
                if (data[i].HasMember("openOrderInitialMargin")) {
                    cFuturePosition.openOrderInitialMargin = std::stod(data[i]["openOrderInitialMargin"].GetString());
                }
                if (data[i].HasMember("leverage")) {
                    cFuturePosition.leverage = std::stod(data[i]["leverage"].GetString());
                }
                if (data[i].HasMember("isolated")) {
                    cFuturePosition.isolated = data[i]["isolated"].GetBool();
                }
                if (data[i].HasMember("entryPrice")) {
                    cFuturePosition.entryPrice = std::stod(data[i]["entryPrice"].GetString());
                }
                if (data[i].HasMember("maxQty")) {
                    cFuturePosition.maxQty = std::stod(data[i]["maxQty"].GetString());
                }
                if (data[i].HasMember("positionSide")) {
                    cFuturePosition.positionSide = data[i]["positionSide"].GetString();
                }
                if (data[i].HasMember("positionAmt")) {
                    /*
                    if (cFuturePosition.positionSide == "SHORT") {
                        cFuturePosition.positionAmt = -stod(position.at("positionAmt").as_string());
                    } else {
                        cFuturePosition.positionAmt = stod(position.at("positionAmt").as_string());
                    }
                    */
                    cFuturePosition.positionAmt = std::stod(data[i]["positionAmt"].GetString());
                }
                if (data[i].HasMember("updateTime")) {
                    cFuturePosition.updateTime = std::stoll(data[i]["updateTime"].GetString());
                }
        
                if (fabs(cFuturePosition.positionAmt) <= 0.0000000001) {
                    continue;
                }

                LOG_INFO("QueryAccount AccountId: {}   cFuturePosition: {}", accountInfo.accountId, cFuturePosition.toString());
                vCFuturePosition.emplace_back(cFuturePosition);

            }
        }

        if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceCFuture QueryAccount code: {}, msg: {}", code, msg);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceCFuture QueryAccount exception: {}", e.what());
        LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool BinanceCFuture::QueryPositionRisk(std::vector<binance::PositionRisk>& vPositionRisk, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", positionRiskUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(baseUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceCFuture QueryPositionRisk syncGet return false";
            LOG_INFO("QueryPositionRisk AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceCFuture QueryPositionRisk status: {}", status);
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

                LOG_INFO("QueryPositionRisk AccountId: {}   cFuturePosition: {}", accountInfo.accountId, positionRisk.toString());
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

            std::string errMsg = fmt::format("BinanceCFuture QueryPositionRisk code: {}, msg: {}", code, msg);
            LOG_INFO("QueryPositionRisk AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceCFuture QueryPositionRisk exception: {}", e.what());
        LOG_INFO("QueryPositionRisk AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool BinanceCFuture::QueryOpenOrder(std::vector<binance::FutureOpenOrder>& vOpenOrder, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", openOrderUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(baseUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceCFuture QueryOpenOrder syncGet return false";
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceCFuture QueryOpenOrder status: {}", status);
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
                LOG_INFO("BinanceCFuture AccountId: {}   QueryOpenOrder: {}", accountInfo.accountId, openOrder.toString());
            }
        }

        if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceCFuture QueryOpenOrder code: {}, msg: {}", code, msg);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceCFuture QueryOpenOrder exception: {}", e.what());
        LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}
