#include "binance/BinanceUnify.h"


BinanceUnify::BinanceUnify(AccountInfo& info) {
    baseUrl = "https://papi.binance.com";
    balanceUrl = "/papi/v1/balance";
    accountUrl = "/papi/v1/account";
    umPositionUrl = "/papi/v1/um/positionRisk";
    cmPositionUrl = "/papi/v1/cm/positionRisk";
    umOpenOrderUrl = "/papi/v1/um/openOrders";
    cmOpenOrderUrl = "/papi/v1/cm/openOrders";
    accountInfo = info;
}

BinanceUnify::~BinanceUnify() {
}

bool BinanceUnify::QueryBalance(vector<binance::UnifyAsset>& vUnifyAsset, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", balanceUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceUnify QueryBalance syncGet return false";
            LOG_INFO("QueryBalance AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceUnify QueryBalance status: {}", status);
            LOG_INFO("QueryBalance AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                binance::UnifyAsset unifyAsset;
                if (res[i].HasMember("asset")) {
                    unifyAsset.asset = res[i]["asset"].GetString();
                }
                if (res[i].HasMember("totalWalletBalance")) {
                    unifyAsset.totalWalletBalance = std::stod(res[i]["totalWalletBalance"].GetString());
                }
                if (res[i].HasMember("crossMarginAsset")) {
                    unifyAsset.crossMarginAsset = std::stod(res[i]["crossMarginAsset"].GetString());
                }
                if (res[i].HasMember("crossMarginBorrowed")) {
                    unifyAsset.crossMarginBorrowed = std::stod(res[i]["crossMarginBorrowed"].GetString());
                }
                if (res[i].HasMember("crossMarginFree")) {
                    unifyAsset.crossMarginFree = std::stod(res[i]["crossMarginFree"].GetString());
                }
                if (res[i].HasMember("crossMarginInterest")) {
                    unifyAsset.crossMarginInterest = std::stod(res[i]["crossMarginInterest"].GetString());
                }
                if (res[i].HasMember("crossMarginLocked")) {
                    unifyAsset.crossMarginLocked = std::stod(res[i]["crossMarginLocked"].GetString());
                }
                if (res[i].HasMember("crossMarginNetAsset")) {
                    unifyAsset.crossMarginNetAsset = std::stod(res[i]["crossMarginNetAsset"].GetString());
                }
                if (res[i].HasMember("umWalletBalance")) {
                    unifyAsset.umWalletBalance = std::stod(res[i]["umWalletBalance"].GetString());
                }
                if (res[i].HasMember("umUnrealizedPNL")) {
                    unifyAsset.umUnrealizedPnl = std::stod(res[i]["umUnrealizedPNL"].GetString());
                }
                if (res[i].HasMember("cmWalletBalance")) {
                    unifyAsset.cmWalletBalance = std::stod(res[i]["cmWalletBalance"].GetString());
                }
                if (res[i].HasMember("cmUnrealizedPNL")) {
                    unifyAsset.cmUnrealizedPnl = std::stod(res[i]["cmUnrealizedPNL"].GetString());
                }
                if (res[i].HasMember("updateTime")) {
                    unifyAsset.updateTime = std::stoll(res[i]["updateTime"].GetString());
                }

                if (fabs(unifyAsset.totalWalletBalance) <= 0.0000000001) {
                    continue;
                }

                LOG_INFO("QueryBalance AccountId: {}  unifyAsset: {}", accountInfo.accountId, unifyAsset.toString());
                vUnifyAsset.emplace_back(unifyAsset);
            }
        }
        else if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceUnify QueryBalance code: {}, msg: {}", code, msg);
            LOG_INFO("QueryBalance AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceUnify QueryBalance exception: {}", e.what());
        LOG_INFO("QueryBalance AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool BinanceUnify::QueryAccount(binance::UnifyAccount& unifyAccount, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", accountUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceUnify QueryAccount syncGet return false";
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceUnify QueryAccount status: {}", status);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.HasMember("uniMMR")) {
            unifyAccount.uniMMR = std::stod(res["uniMMR"].GetString());
        }
        if (res.HasMember("accountEquity")) {
            unifyAccount.accountEquity = std::stod(res["accountEquity"].GetString());
        }
        if (res.HasMember("actualEquity")) {
            unifyAccount.actualEquity = std::stod(res["actualEquity"].GetString());
        }
        if (res.HasMember("accountInitialMargin")) {
            unifyAccount.accountInitialMargin = std::stod(res["accountInitialMargin"].GetString());
        }
        if (res.HasMember("accountMaintMargin")) {
            unifyAccount.accountMaintMargin = std::stod(res["accountMaintMargin"].GetString());
        }
        if (res.HasMember("accountStatus")) {
            unifyAccount.accountStatus = res["accountStatus"].GetString();
        }
        if (res.HasMember("virtualMaxWithdrawAmount")) {
            unifyAccount.virtualMaxWithdrawAmount = std::stod(res["virtualMaxWithdrawAmount"].GetString());
        }
        if (res.HasMember("totalAvailableBalance")) {
            unifyAccount.totalAvailableBalance = std::stod(res["totalAvailableBalance"].GetString());
        }
        if (res.HasMember("totalMarginOpenLoss")) {
            unifyAccount.totalMarginOpenLoss = std::stod(res["totalMarginOpenLoss"].GetString());
        }
        if (res.HasMember("updateTime")) {
            unifyAccount.updateTime = std::stoll(res["updateTime"].GetString());
        }

        if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceUnify QueryAccount code: {}, msg: {}", code, msg);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceUnify QueryAccount exception: {}", e.what());
        LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool BinanceUnify::QueryUmPosition(vector<binance::UnifyPosition>& vUnifyPosition, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", umPositionUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(baseUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceUnify QueryUmPosition syncGet return false";
            LOG_INFO("QueryUmPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceUnify QueryUmPosition status: {}", status);
            LOG_INFO("QueryUmPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value& res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

         if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                binance::UnifyPosition unifyPosition;
                if (res[i].HasMember("symbol")) {
                    unifyPosition.symbol = res[i]["symbol"].GetString();
                }
                if (res[i].HasMember("entryPrice")) {
                    unifyPosition.entryPrice = std::stod(res[i]["entryPrice"].GetString(););
                }
                if (res[i].HasMember("leverage")) {
                    unifyPosition.leverage = std::stod(res[i]["leverage"].GetString(););
                }
                if (res[i].HasMember("markPrice")) {
                    unifyPosition.markPrice = std::stod(res[i]["markPrice"].GetString(););
                }
                if (res[i].HasMember("maxNotionalValue")) {
                    unifyPosition.maxNotionalValue = std::stod(res[i]["maxNotionalValue"].GetString(););
                }
                if (res[i].HasMember("positionAmt")) {
                    unifyPosition.positionAmt = std::stod(res[i]["positionAmt"].GetString(););
                }
                if (res[i].HasMember("notional")) {
                    unifyPosition.notional = std::stod(res[i]["notional"].GetString(););
                }
                if (res[i].HasMember("unRealizedProfit")) {
                    unifyPosition.unRealizedProfit = std::stod(res[i]["unRealizedProfit"].GetString(););
                }
                if (res[i].HasMember("positionSide")) {
                    unifyPosition.positionSide = res[i]["positionSide"].GetString();;
                }
                if (res[i].HasMember("updateTime")) {
                    unifyPosition.updateTime = std::stoll(res[i]["updateTime"].GetString(););
                }

                if (fabs(unifyPosition.positionAmt) <= 0.0000000001) {
                    continue;
                }

                LOG_INFO("QueryUmPosition AccountId: {}    unifyPosition: {}", accountInfo.accountId, unifyPosition.toString());
                vUnifyPosition.emplace_back(unifyPosition);
            }
        }

        if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceUnify QueryUmPosition code: {}, msg: {}", code, msg);
            LOG_INFO("QueryUmPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceUnify QueryUmPosition exception: {}", e.what());
        LOG_INFO("QueryUmPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;  
}

bool BinanceUnify::QueryCmPosition(vector<binance::UnifyPosition>& vUnifyPosition, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", cmPositionUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(baseUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceUnify QueryCmPosition syncGet return false";
            LOG_INFO("QueryCmPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceUnify QueryCmPosition status: {}", status);
            LOG_INFO("QueryCmPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value& res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

         if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                binance::UnifyPosition unifyPosition;
                if (res[i].HasMember("symbol")) {
                    unifyPosition.symbol = res[i]["symbol"].GetString();
                }
                if (res[i].HasMember("entryPrice")) {
                    unifyPosition.entryPrice = std::stod(res[i]["entryPrice"].GetString(););
                }
                if (res[i].HasMember("leverage")) {
                    unifyPosition.leverage = std::stod(res[i]["leverage"].GetString(););
                }
                if (res[i].HasMember("markPrice")) {
                    unifyPosition.markPrice = std::stod(res[i]["markPrice"].GetString(););
                }
                if (res[i].HasMember("maxNotionalValue")) {
                    unifyPosition.maxNotionalValue = std::stod(res[i]["maxNotionalValue"].GetString(););
                }
                if (res[i].HasMember("positionAmt")) {
                    unifyPosition.positionAmt = std::stod(res[i]["positionAmt"].GetString(););
                }
                if (res[i].HasMember("notional")) {
                    unifyPosition.notional = std::stod(res[i]["notional"].GetString(););
                }
                if (res[i].HasMember("unRealizedProfit")) {
                    unifyPosition.unRealizedProfit = std::stod(res[i]["unRealizedProfit"].GetString(););
                }
                if (res[i].HasMember("positionSide")) {
                    unifyPosition.positionSide = res[i]["positionSide"].GetString();;
                }
                if (res[i].HasMember("updateTime")) {
                    unifyPosition.updateTime = std::stoll(res[i]["updateTime"].GetString(););
                }

                if (fabs(unifyPosition.positionAmt) <= 0.0000000001) {
                    continue;
                }

                LOG_INFO("QueryCmPosition AccountId: {}    unifyPosition: {}", accountInfo.accountId, unifyPosition.toString());
                vUnifyPosition.emplace_back(unifyPosition);
            }
        }

        if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceUnify QueryCmPosition code: {}, msg: {}", code, msg);
            LOG_INFO("QueryCmPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceUnify QueryCmPosition exception: {}", e.what());
        LOG_INFO("QueryCmPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool BinanceUnify::QueryUmOpenOrder(vector<binance::UnifyOpenOrder>& vOpenOrder, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", umOpenOrderUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(baseUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceUnify QueryUmOpenOrder syncGet return false";
            LOG_INFO("QueryUmOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceUnify QueryUmOpenOrder status: {}", status);
            LOG_INFO("QueryUmOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value& res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

         if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                binance::UnifyOpenOrder openOrder;
                if (res[i].HasMember("symbol")) {
                    openOrder.symbol = res[i]["symbol"].GetString();
                }
                if (res[i].HasMember("orderId")) {
                    openOrder.orderId = std::stoll(res[i]["orderId"].GetString();)
                }
                if (res[i].HasMember("clientOrderId")) {
                    openOrder.clientOrderId = res[i]["clientOrderId"].GetString();
                }
                if (res[i].HasMember("price")) {
                    openOrder.price = stod(res[i]["price"].GetString(););
                }
                if (res[i].HasMember("origQty")) {
                    openOrder.origQty = stod(res[i]["origQty"].GetString(););
                }
                if (res[i].HasMember("executedQty")) {
                    openOrder.executedQty = stod(res[i]["executedQty"].GetString(););
                }
                if (res[i].HasMember("avgPrice")) {
                    openOrder.avgPrice = stod(res[i]["avgPrice"].GetString(););
                }
                if (res[i].HasMember("cumQuote")) {
                    openOrder.cumQuote = stod(res[i]["cumQuote"].GetString(););
                }
                if (res[i].HasMember("side")) {
                    openOrder.side = res[i]["side"].GetString();
                }
                if (res[i].HasMember("positionSide")) {
                    openOrder.positionSide = res[i]["positionSide"].GetString();
                }
                if (res[i].HasMember("udpateTime")) {
                    openOrder.time = std::stoll(res[i]["udpateTime"].GetString(););
                }
            
                LOG_INFO("BinanceUnify AccountId: {}  QueryUmOpenOrder: {}", accountInfo.accountId, openOrder.toString());
                vOpenOrder.emplace_back(openOrder);
            }
        }

        if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceUnify QueryUmOpenOrder code: {}, msg: {}", code, msg);
            LOG_INFO("QueryUmOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceUnify QueryUmOpenOrder exception: {}", e.what());
        LOG_INFO("QueryUmOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool BinanceUnify::QueryCmOpenOrder(vector<binance::UnifyOpenOrder>& vOpenOrder, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature=", cmOpenOrderUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(baseUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceUnify QueryCmOpenOrder syncGet return false";
            LOG_INFO("QueryCmOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceUnify QueryCmOpenOrder status: {}", status);
            LOG_INFO("QueryCmOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value& res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

         if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                binance::UnifyOpenOrder openOrder;
                if (res[i].HasMember("symbol")) {
                    openOrder.symbol = res[i]["symbol"].GetString();
                }
                if (res[i].HasMember("orderId")) {
                    openOrder.orderId = std::stoll(res[i]["orderId"].GetString();)
                }
                if (res[i].HasMember("clientOrderId")) {
                    openOrder.clientOrderId = res[i]["clientOrderId"].GetString();
                }
                if (res[i].HasMember("price")) {
                    openOrder.price = stod(res[i]["price"].GetString(););
                }
                if (res[i].HasMember("origQty")) {
                    openOrder.origQty = stod(res[i]["origQty"].GetString(););
                }
                if (res[i].HasMember("executedQty")) {
                    openOrder.executedQty = stod(res[i]["executedQty"].GetString(););
                }
                if (res[i].HasMember("avgPrice")) {
                    openOrder.avgPrice = stod(res[i]["avgPrice"].GetString(););
                }
                if (res[i].HasMember("cumQuote")) {
                    openOrder.cumQuote = stod(res[i]["cumQuote"].GetString(););
                }
                if (res[i].HasMember("side")) {
                    openOrder.side = res[i]["side"].GetString();
                }
                if (res[i].HasMember("positionSide")) {
                    openOrder.positionSide = res[i]["positionSide"].GetString();
                }
                if (res[i].HasMember("udpateTime")) {
                    openOrder.time = std::stoll(res[i]["udpateTime"].GetString(););
                }
            
                LOG_INFO("BinanceUnify AccountId: {}  QueryCmOpenOrder: {}", accountInfo.accountId, openOrder.toString());
                vOpenOrder.emplace_back(openOrder);
            }
        }

        if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceUnify QueryCmOpenOrder code: {}, msg: {}", code, msg);
            LOG_INFO("QueryCmOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceUnify QueryCmOpenOrder exception: {}", e.what());
        LOG_INFO("QueryCmOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}
