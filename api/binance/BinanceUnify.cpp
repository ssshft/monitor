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
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vUnifyAsset.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(baseUrl, config);
            http_request request(methods::GET);
            request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
            uri_builder builder(balanceUrl);

            builder.append_query("recvWindow", 5000);
            builder.append_query("timestamp", gettickcount());
            auto signature = getSignature(builder.query(), accountInfo.secretKey);
            builder.append_query("signature", signature);    

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([&](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if (code == status_codes::OK) {
                    return response.extract_json();
                } else if (code >= status_codes::BadRequest && code < status_codes::InternalError) {
                    auto content_type = response.headers().content_type();
                    if (content_type.find("application/json") >= 0) {
                        return response.extract_json();
                    }
                }

                LOG_INFO("QueryBalance AccountId: %d  BinanceUnify response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& assets = previousTask.get();
                if (assets.is_array()) {
                    auto assetArray = assets.as_array();
                    for (auto& asset : assetArray) {
                        binance::UnifyAsset unifyAsset;
                        if (asset.has_field("asset")) {
                            unifyAsset.asset = asset.at("asset").as_string();
                        }
                        if (asset.has_field("totalWalletBalance")) {
                            unifyAsset.totalWalletBalance = stod(asset.at("totalWalletBalance").as_string());
                        }
                        if (asset.has_field("crossMarginAsset")) {
                            unifyAsset.crossMarginAsset = stod(asset.at("crossMarginAsset").as_string());
                        }
                        if (asset.has_field("crossMarginBorrowed")) {
                            unifyAsset.crossMarginBorrowed = stod(asset.at("crossMarginBorrowed").as_string());
                        }
                        if (asset.has_field("crossMarginFree")) {
                            unifyAsset.crossMarginFree = stod(asset.at("crossMarginFree").as_string());
                        }
                        if (asset.has_field("crossMarginInterest")) {
                            unifyAsset.crossMarginInterest = stod(asset.at("crossMarginInterest").as_string());
                        }
                        if (asset.has_field("crossMarginLocked")) {
                            unifyAsset.crossMarginLocked = stod(asset.at("crossMarginLocked").as_string());
                        }
                        if (asset.has_field("crossMarginNetAsset")) {
                            unifyAsset.crossMarginNetAsset = stod(asset.at("crossMarginNetAsset").as_string());
                        }
                        if (asset.has_field("umWalletBalance")) {
                            unifyAsset.umWalletBalance = stod(asset.at("umWalletBalance").as_string());
                        }
                        if (asset.has_field("umUnrealizedPNL")) {
                            unifyAsset.umUnrealizedPnl = stod(asset.at("umUnrealizedPNL").as_string());
                        }
                        if (asset.has_field("cmWalletBalance")) {
                            unifyAsset.cmWalletBalance = stod(asset.at("cmWalletBalance").as_string());
                        }
                        if (asset.has_field("cmUnrealizedPNL")) {
                            unifyAsset.cmUnrealizedPnl = stod(asset.at("cmUnrealizedPNL").as_string());
                        }
                        if (asset.has_field("updateTime")) {
                            unifyAsset.updateTime = asset.at("updateTime").as_number().to_int64();
                        }

                        if (fabs(unifyAsset.totalWalletBalance) <= 0.0000000001) {
                            continue;
                        }

                        LOG_INFO("QueryAccount AccountId: %d    unifyAsset: %s", accountInfo.accountId, unifyAsset.toString().c_str());
                        vUnifyAsset.emplace_back(unifyAsset);
                    }
                }
             

                if (assets.has_field("code")) {
                    query = false;
                    stringstream ss;
                    int code = assets.at("code").as_integer();
                    string msg = "";
                    if (assets.has_field("msg")) {
                        msg = assets.at("msg").as_string();
                    }
                    ss << "BinanceUnify QueryAccount code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceUnify QueryAccount ") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }

        if (query) {
            break;
        }
        usleep(1);
        count++;
    }
    return query;
}

bool BinanceUnify::QueryAccount(binance::UnifyAccount& unifyAccount, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(baseUrl, config);
            http_request request(methods::GET);
            request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
            uri_builder builder(accountUrl);

            builder.append_query("recvWindow", 5000);
            builder.append_query("timestamp", gettickcount());
            auto signature = getSignature(builder.query(), accountInfo.secretKey);
            builder.append_query("signature", signature);    

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([&](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if (code == status_codes::OK) {
                    return response.extract_json();
                } else if (code >= status_codes::BadRequest && code < status_codes::InternalError) {
                    auto content_type = response.headers().content_type();
                    if (content_type.find("application/json") >= 0) {
                        return response.extract_json();
                    }
                }

                LOG_INFO("QueryAccount AccountId: %d  BinanceUnify response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.has_field("uniMMR")) {
                    unifyAccount.uniMMR = stod(content.at("uniMMR").as_string());
                }
                if (content.has_field("accountEquity")) {
                    unifyAccount.accountEquity = stod(content.at("accountEquity").as_string());
                }
                if (content.has_field("actualEquity")) {
                    unifyAccount.actualEquity = stod(content.at("actualEquity").as_string());
                }
                if (content.has_field("accountInitialMargin")) {
                    unifyAccount.accountInitialMargin = stod(content.at("accountInitialMargin").as_string());
                }
                if (content.has_field("accountMaintMargin")) {
                    unifyAccount.accountMaintMargin = stod(content.at("accountMaintMargin").as_string());
                }
                if (content.has_field("accountStatus")) {
                    unifyAccount.accountStatus = content.at("accountStatus").as_string();
                }
                if (content.has_field("virtualMaxWithdrawAmount")) {
                    unifyAccount.virtualMaxWithdrawAmount = stod(content.at("virtualMaxWithdrawAmount").as_string());
                }
                if (content.has_field("totalAvailableBalance")) {
                    unifyAccount.totalAvailableBalance = stod(content.at("totalAvailableBalance").as_string());
                }
                if (content.has_field("totalMarginOpenLoss")) {
                    unifyAccount.totalMarginOpenLoss = stod(content.at("totalMarginOpenLoss").as_string());
                }
                if (content.has_field("updateTime")) {
                    unifyAccount.updateTime = content.at("updateTime").as_number().to_int64();
                }
                
                LOG_INFO("QueryAccount AccountId: %d    UnifyAccount: %s", accountInfo.accountId, unifyAccount.toString().c_str());

                if (content.has_field("code")) {
                    query = false;
                    stringstream ss;
                    int code = content.at("code").as_integer();
                    string msg = "";
                    if (content.has_field("msg")) {
                        msg = content.at("msg").as_string();
                    }
                    ss << "BinanceUnify QueryAccount code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceUnify QueryAccount ") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }

        if (query) {
            break;
        }
        usleep(1);
        count++;
    }
    return query;
}

bool BinanceUnify::QueryUmPosition(vector<binance::UnifyPosition>& vUnifyPosition, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vUnifyPosition.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(baseUrl, config);
            http_request request(methods::GET);
            request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
            uri_builder builder(umPositionUrl);

            builder.append_query("recvWindow", 5000);
            builder.append_query("timestamp", gettickcount());
            auto signature = getSignature(builder.query(), accountInfo.secretKey);
            builder.append_query("signature", signature);    

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([&](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if (code == status_codes::OK) {
                    return response.extract_json();
                } else if (code >= status_codes::BadRequest && code < status_codes::InternalError) {
                    auto content_type = response.headers().content_type();
                    if (content_type.find("application/json") >= 0) {
                        return response.extract_json();
                    }
                }

                LOG_INFO("QueryUmPosition AccountId: %d  BinanceUnify response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& positions = previousTask.get();
                if (positions.is_array()) {
                    auto positionArray = positions.as_array();
                    for (auto& position : positionArray) {
                        binance::UnifyPosition unifyPosition;
                        if (position.has_field("symbol")) {
                            unifyPosition.symbol = position.at("symbol").as_string();
                        }
                        if (position.has_field("entryPrice")) {
                            unifyPosition.entryPrice = stod(position.at("entryPrice").as_string());
                        }
                         if (position.has_field("leverage")) {
                            unifyPosition.leverage = stod(position.at("leverage").as_string());
                        }
                        if (position.has_field("markPrice")) {
                            unifyPosition.markPrice = stod(position.at("markPrice").as_string());
                        }
                        if (position.has_field("maxNotionalValue")) {
                            unifyPosition.maxNotionalValue = stod(position.at("maxNotionalValue").as_string());
                        }
                        if (position.has_field("positionAmt")) {
                            unifyPosition.positionAmt = stod(position.at("positionAmt").as_string());
                        }
                        if (position.has_field("notional")) {
                            unifyPosition.notional = stod(position.at("notional").as_string());
                        }
                        if (position.has_field("unRealizedProfit")) {
                            unifyPosition.unRealizedProfit = stod(position.at("unRealizedProfit").as_string());
                        }
                        if (position.has_field("positionSide")) {
                            unifyPosition.positionSide = position.at("positionSide").as_string();
                        }
                        if (position.has_field("updateTime")) {
                            unifyPosition.updateTime = position.at("updateTime").as_number().to_int64();
                        }

                        if (fabs(unifyPosition.positionAmt) <= 0.0000000001) {
                            continue;
                        }

                        LOG_INFO("QueryUmPosition AccountId: %d    unifyPosition: %s", accountInfo.accountId, unifyPosition.toString().c_str());
                        vUnifyPosition.emplace_back(unifyPosition);
                    }
                }
             

                if (positions.has_field("code")) {
                    query = false;
                    stringstream ss;
                    int code = positions.at("code").as_integer();
                    string msg = "";
                    if (positions.has_field("msg")) {
                        msg = positions.at("msg").as_string();
                    }
                    ss << "BinanceUnify QueryUmPosition code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryUmPosition AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceUnify QueryUmPosition ") + string(e.what());
            LOG_INFO("QueryUmPosition AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }

        if (query) {
            break;
        }
        usleep(1);
        count++;
    }
    return query;
}

bool BinanceUnify::QueryCmPosition(vector<binance::UnifyPosition>& vUnifyPosition, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vUnifyPosition.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(baseUrl, config);
            http_request request(methods::GET);
            request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
            uri_builder builder(cmPositionUrl);

            builder.append_query("recvWindow", 5000);
            builder.append_query("timestamp", gettickcount());
            auto signature = getSignature(builder.query(), accountInfo.secretKey);
            builder.append_query("signature", signature);    

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([&](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if (code == status_codes::OK) {
                    return response.extract_json();
                } else if (code >= status_codes::BadRequest && code < status_codes::InternalError) {
                    auto content_type = response.headers().content_type();
                    if (content_type.find("application/json") >= 0) {
                        return response.extract_json();
                    }
                }

                LOG_INFO("QueryCmPosition AccountId: %d  BinanceUnify response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& positions = previousTask.get();
                if (positions.is_array()) {
                    auto positionArray = positions.as_array();
                    for (auto& position : positionArray) {
                        binance::UnifyPosition unifyPosition;
                        if (position.has_field("symbol")) {
                            unifyPosition.symbol = position.at("symbol").as_string();
                        }
                        if (position.has_field("entryPrice")) {
                            unifyPosition.entryPrice = stod(position.at("entryPrice").as_string());
                        }
                         if (position.has_field("leverage")) {
                            unifyPosition.leverage = stod(position.at("leverage").as_string());
                        }
                        if (position.has_field("markPrice")) {
                            unifyPosition.markPrice = stod(position.at("markPrice").as_string());
                        }
                        if (position.has_field("positionAmt")) {
                            unifyPosition.positionAmt = stod(position.at("positionAmt").as_string());
                        }
                        if (position.has_field("unRealizedProfit")) {
                            unifyPosition.unRealizedProfit = stod(position.at("unRealizedProfit").as_string());
                        }
                        if (position.has_field("positionSide")) {
                            unifyPosition.positionSide = position.at("positionSide").as_string();
                        }
                        if (position.has_field("updateTime")) {
                            unifyPosition.updateTime = position.at("updateTime").as_number().to_int64();
                        }

                        if (fabs(unifyPosition.positionAmt) <= 0.0000000001) {
                            continue;
                        }

                        LOG_INFO("QueryCmPosition AccountId: %d    unifyPosition: %s", accountInfo.accountId, unifyPosition.toString().c_str());
                        vUnifyPosition.emplace_back(unifyPosition);
                    }
                }
             

                if (positions.has_field("code")) {
                    query = false;
                    stringstream ss;
                    int code = positions.at("code").as_integer();
                    string msg = "";
                    if (positions.has_field("msg")) {
                        msg = positions.at("msg").as_string();
                    }
                    ss << "BinanceUnify QueryCmPosition code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryCmPosition AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceUnify QueryCmPosition ") + string(e.what());
            LOG_INFO("QueryCmPosition AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }

        if (query) {
            break;
        }
        usleep(1);
        count++;
    }
    return query;
}

bool BinanceUnify::QueryUmOpenOrder(vector<binance::UnifyOpenOrder>& vOpenOrder, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vOpenOrder.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(baseUrl, config);
            http_request request(methods::GET);
            request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
            uri_builder builder(umOpenOrderUrl);

            int64_t currentTime = gettickcount();
            builder.append_query("recvWindow", 5000);
            builder.append_query("timestamp", currentTime);
            auto signature = getSignature(builder.query(), accountInfo.secretKey);
            builder.append_query("signature", signature);    

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([&](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if (code == status_codes::OK) {
                    return response.extract_json();
                } else if (code >= status_codes::BadRequest && code < status_codes::InternalError) {
                    auto content_type = response.headers().content_type();
                    if (content_type.find("application/json") >= 0) {
                        return response.extract_json();
                    }
                }

                LOG_INFO("QueryUmOpenOrder AccountId: %d  BinanceUnify response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.is_array()) {
                    auto openOrderArray = content.as_array();
                    for (auto& arr : openOrderArray) {
                        binance::UnifyOpenOrder openOrder;
                        if (arr.has_field("symbol")) {
                            openOrder.symbol = arr.at("symbol").as_string();
                        }
                        if (arr.has_field("orderId")) {
                            openOrder.orderId = arr.at("orderId").as_number().to_int64();
                        }
                        if (arr.has_field("clientOrderId")) {
                            openOrder.clientOrderId = arr.at("clientOrderId").as_string();
                        }
                        if (arr.has_field("price")) {
                            openOrder.price = stod(arr.at("price").as_string());
                        }
                        if (arr.has_field("origQty")) {
                            openOrder.origQty = stod(arr.at("origQty").as_string());
                        }
                        if (arr.has_field("executedQty")) {
                            openOrder.executedQty = stod(arr.at("executedQty").as_string());
                        }
                        if (arr.has_field("avgPrice")) {
                            openOrder.avgPrice = stod(arr.at("avgPrice").as_string());
                        }
                        if (arr.has_field("cumQuote")) {
                            openOrder.cumQuote = stod(arr.at("cumQuote").as_string());
                        }
                        if (arr.has_field("side")) {
                            openOrder.side = arr.at("side").as_string();
                        }
                        if (arr.has_field("positionSide")) {
                            openOrder.positionSide = arr.at("positionSide").as_string();
                        }
                        if (arr.has_field("udpateTime")) {
                            openOrder.time = arr.at("udpateTime").as_number().to_int64();
                        }
                    
                        vOpenOrder.emplace_back(openOrder);
                    	LOG_INFO("BinanceUnify AccountId: %d  QueryUmOpenOrder: %s", accountInfo.accountId, openOrder.toString().c_str());
                    }
                }

                if (content.has_field("code")) {
                    query = false;
                    stringstream ss;
                    int code = content.at("code").as_integer();
                    string msg = "";
                    if (content.has_field("msg")) {
                        msg = content.at("msg").as_string();
                    }
                    ss << "BinanceUnify QueryUmOpenOrder code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryUmOpenOrder AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceUnify QueryUmOpenOrder ") + string(e.what());
            LOG_INFO("QueryUmOpenOrder AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }

        if (query) {
            break;
        }
        usleep(1);
        count++;
    }
    return query;
}

bool BinanceUnify::QueryCmOpenOrder(vector<binance::UnifyOpenOrder>& vOpenOrder, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vOpenOrder.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(baseUrl, config);
            http_request request(methods::GET);
            request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
            uri_builder builder(cmOpenOrderUrl);

            int64_t currentTime = gettickcount();
            builder.append_query("recvWindow", 5000);
            builder.append_query("timestamp", currentTime);
            auto signature = getSignature(builder.query(), accountInfo.secretKey);
            builder.append_query("signature", signature);    

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([&](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if (code == status_codes::OK) {
                    return response.extract_json();
                } else if (code >= status_codes::BadRequest && code < status_codes::InternalError) {
                    auto content_type = response.headers().content_type();
                    if (content_type.find("application/json") >= 0) {
                        return response.extract_json();
                    }
                }

                LOG_INFO("QueryCmOpenOrder AccountId: %d  BinanceUnify response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.is_array()) {
                    auto openOrderArray = content.as_array();
                    for (auto& arr : openOrderArray) {
                        binance::UnifyOpenOrder openOrder;
                        if (arr.has_field("symbol")) {
                            openOrder.symbol = arr.at("symbol").as_string();
                        }
                        if (arr.has_field("orderId")) {
                            openOrder.orderId = arr.at("orderId").as_number().to_int64();
                        }
                        if (arr.has_field("clientOrderId")) {
                            openOrder.clientOrderId = arr.at("clientOrderId").as_string();
                        }
                        if (arr.has_field("price")) {
                            openOrder.price = stod(arr.at("price").as_string());
                        }
                        if (arr.has_field("origQty")) {
                            openOrder.origQty = stod(arr.at("origQty").as_string());
                        }
                        if (arr.has_field("executedQty")) {
                            openOrder.executedQty = stod(arr.at("executedQty").as_string());
                        }
                        if (arr.has_field("avgPrice")) {
                            openOrder.avgPrice = stod(arr.at("avgPrice").as_string());
                        }
                        if (arr.has_field("cumQuote")) {
                            openOrder.cumQuote = stod(arr.at("cumQuote").as_string());
                        }
                        if (arr.has_field("side")) {
                            openOrder.side = arr.at("side").as_string();
                        }
                        if (arr.has_field("positionSide")) {
                            openOrder.positionSide = arr.at("positionSide").as_string();
                        }
                        if (arr.has_field("udpateTime")) {
                            openOrder.time = arr.at("udpateTime").as_number().to_int64();
                        }
                    
                        vOpenOrder.emplace_back(openOrder);
                    	LOG_INFO("BinanceUnify AccountId: %d  QueryCmOpenOrder: %s", accountInfo.accountId, openOrder.toString().c_str());
                    }
                }

                if (content.has_field("code")) {
                    query = false;
                    stringstream ss;
                    int code = content.at("code").as_integer();
                    string msg = "";
                    if (content.has_field("msg")) {
                        msg = content.at("msg").as_string();
                    }
                    ss << "BinanceUnify QueryCmOpenOrder code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryCmOpenOrder AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceUnify QueryCmOpenOrder ") + string(e.what());
            LOG_INFO("QueryCmOpenOrder AccountId: %d   BinanceUnify Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }

        if (query) {
            break;
        }
        usleep(1);
        count++;
    }
    return query;
}
