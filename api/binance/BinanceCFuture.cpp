#include "BinanceCFuture.h"


BinanceCFuture::BinanceCFuture(AccountInfo& info) {
    baseUrl = "https://dapi.binance.com";
    accountUrl = "/dapi/v1/account";
    tradesUrl = "/dapi/v1/userTrades";
    positionRiskUrl = "/dapi/v1/positionRisk";
    openOrderUrl = "/dapi/v1/openOrders";
    accountInfo = info;
}

BinanceCFuture::~BinanceCFuture() {
}

bool BinanceCFuture::QueryAccount(vector<binance::CFutureAsset>& vCFutureAsset, vector<binance::CFuturePosition>& vCFuturePosition, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vCFutureAsset.clear();
        vCFuturePosition.clear();
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

                LOG_INFO("QueryAccount AccountId: %d  BinanceCFuture response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.has_field("assets")) {
                    json::value const& assets = content.at("assets");
                    if (assets.is_array()) {
                        auto assetArray = assets.as_array();
                        for(auto& asset : assetArray) {
                            binance::CFutureAsset cFutureAsset;
                            if (asset.has_field("asset")) {
                                cFutureAsset.assetType = asset.at("asset").as_string();
                            }
                            if (asset.has_field("walletBalance")) {
                                cFutureAsset.walletBalance = stod(asset.at("walletBalance").as_string());
                            }
                            if (asset.has_field("unrealizedProfit")) {
                                cFutureAsset.unrealizedProfit = stod(asset.at("unrealizedProfit").as_string());
                            }
                            if (asset.has_field("marginBalance")) {
                                cFutureAsset.marginBalance = stod(asset.at("marginBalance").as_string());
                            }
                            if (asset.has_field("maintMargin")) {
                                cFutureAsset.maintMargin = stod(asset.at("maintMargin").as_string());
                            }
                            if (asset.has_field("initialMargin")) {
                                cFutureAsset.initialMargin = stod(asset.at("initialMargin").as_string());
                            }
                            if (asset.has_field("positionInitialMargin")) {
                                cFutureAsset.positionInitialMargin = stod(asset.at("positionInitialMargin").as_string());
                            }
                            if (asset.has_field("openOrderInitialMargin")) {
                                cFutureAsset.openOrderInitialMargin = stod(asset.at("openOrderInitialMargin").as_string());
                            }
                            if (asset.has_field("crossWalletBalance")) {
                                cFutureAsset.crossWalletBalance = stod(asset.at("crossWalletBalance").as_string());
                            }
                            if (asset.has_field("crossUnPnl")) {
                                cFutureAsset.crossUnPnl = stod(asset.at("crossUnPnl").as_string());
                            }
                            if (asset.has_field("availableBalance")) {
                                cFutureAsset.availableBalance = stod(asset.at("availableBalance").as_string());
                            }
                            if (asset.has_field("maxWithdrawAmount")) {
                                cFutureAsset.maxWithdrawAmount = stod(asset.at("maxWithdrawAmount").as_string());
                            }
                        
                            if (fabs(cFutureAsset.walletBalance) <= 0.0000000001) {
                                continue;
                            }

                            LOG_INFO("QueryAccount AccountId: %d    cFutureAsset: %s", accountInfo.accountId, cFutureAsset.toString().c_str());
                            vCFutureAsset.emplace_back(cFutureAsset);
                        }
                    }
                }

                if (content.has_field("positions")) {
                    json::value const& positions = content.at("positions");
                    if (positions.is_array()) {
                        auto positionArray = positions.as_array();
                        for (auto& position : positionArray) {
                            binance::CFuturePosition cFuturePosition;
                            if (position.has_field("symbol")) {
                                cFuturePosition.symbol = position.at("symbol").as_string();
                            }
                            if (position.has_field("initialMargin")) {
                                cFuturePosition.initialMargin = stod(position.at("initialMargin").as_string());
                            }
                            if (position.has_field("maintMargin")) {
                                cFuturePosition.maintMargin = stod(position.at("maintMargin").as_string());
                            }
                            if (position.has_field("unrealizedProfit")) {
                                cFuturePosition.unrealizedProfit = stod(position.at("unrealizedProfit").as_string());
                            }
                            if (position.has_field("positionInitialMargin")) {
                                cFuturePosition.positionInitialMargin = stod(position.at("positionInitialMargin").as_string());
                            }
                            if (position.has_field("openOrderInitialMargin")) {
                                cFuturePosition.openOrderInitialMargin = stod(position.at("openOrderInitialMargin").as_string());
                            }
                            if (position.has_field("leverage")) {
                                cFuturePosition.leverage = stod(position.at("leverage").as_string());
                            }
                            if (position.has_field("isolated")) {
                                cFuturePosition.isolated = position.at("isolated").as_bool();
                            }
                            if (position.has_field("entryPrice")) {
                                cFuturePosition.entryPrice = stod(position.at("entryPrice").as_string());
                            }
                            if (position.has_field("maxQty")) {
                                cFuturePosition.maxQty = stod(position.at("maxQty").as_string());
                            }
                            if (position.has_field("positionSide")) {
                                cFuturePosition.positionSide = position.at("positionSide").as_string();
                            }
                            if (position.has_field("positionAmt")) {
                                /*
                                if (cFuturePosition.positionSide == "SHORT") {
                                    cFuturePosition.positionAmt = -stod(position.at("positionAmt").as_string());
                                } else {
                                    cFuturePosition.positionAmt = stod(position.at("positionAmt").as_string());
                                }
                                */
                                cFuturePosition.positionAmt = stod(position.at("positionAmt").as_string());
                            }
                            if (position.has_field("updateTime")) {
                                cFuturePosition.updateTime = position.at("updateTime").as_number().to_int64();
                            }
                    
                            if (fabs(cFuturePosition.positionAmt) <= 0.0000000001) {
                                continue;
                            }

                            LOG_INFO("QueryAccount AccountId: %d    cFuturePosition: %s", accountInfo.accountId, cFuturePosition.toString().c_str());
                            vCFuturePosition.emplace_back(cFuturePosition);
                        }
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
                    ss << "BinanceCFuture QueryAccount code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   BinanceCFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }

            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "BinanceCFuture QueryAccount code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryAccount AccountId: %d   BinanceCFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
        */
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceCFuture QueryAccount ") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   BinanceCFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

vector<binance::CFutureTrade> BinanceCFuture::QueryTrades(string symbol, int days) {
    vector<binance::CFutureTrade> v;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(baseUrl, config);
        http_request request(methods::GET);
        request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
        uri_builder builder(tradesUrl);

        int64_t currentTime = gettickcount();
        
        builder.append_query("symbol", symbol);
        if (days > 0) {
           int64_t startTime = currentTime - days * 24 * 60 * 60 * 1000; 
           builder.append_query("startTime", startTime);
        }
        builder.append_query("recvWindow", 5000);
        builder.append_query("timestamp", currentTime);
        auto signature = getSignature(builder.query(), accountInfo.secretKey);
        builder.append_query("signature", signature);    

        //LOG_INFO("BinanceCFuture QueryTrades: '%s' ", builder.to_string().c_str());

        request.set_request_uri(builder.to_string());
        client.request(request)
        .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
            auto code = response.status_code();
            if(code == status_codes::OK || code == status_codes::BadRequest) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                return response.extract_json();
            }
                
            LOG_INFO("BinanceCFuture response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();
            if (content.is_array()) {
                auto tradeArray = content.as_array();
                for(auto& trade : tradeArray) {
                    binance::CFutureTrade cFutureTrade;
                    if (trade.has_field("symbol")) {
                        cFutureTrade.symbol = trade.at("symbol").as_string();
                    }
                    if (trade.has_field("id")) {
                        cFutureTrade.id = trade.at("id").as_number().to_int64();
                    }
                    if (trade.has_field("orderId")) {
                        cFutureTrade.orderId = trade.at("orderId").as_number().to_int64();
                    }
                    if (trade.has_field("pair")) {
                        cFutureTrade.pair = trade.at("pair").as_string();
                    }
                    if (trade.has_field("side")) {
                        cFutureTrade.side = trade.at("side").as_string();
                    }
                    if (trade.has_field("price")) {
                        cFutureTrade.price = stod(trade.at("price").as_string());
                    }
                    if (trade.has_field("qty")) {
                        cFutureTrade.qty = stod(trade.at("qty").as_string());
                    }
                    if (trade.has_field("realizedPnl")) {
                        cFutureTrade.realizedPnl = stod(trade.at("realizedPnl").as_string());
                    }
                    if (trade.has_field("marginAsset")) {
                        cFutureTrade.marginAsset = trade.at("marginAsset").as_string();
                    }
                    if (trade.has_field("baseQty")) {
                        cFutureTrade.baseQty = stod(trade.at("baseQty").as_string());
                    }
                    if (trade.has_field("commission")) {
                        cFutureTrade.commission = stod(trade.at("commission").as_string());
                    }
                    if (trade.has_field("commissionAsset")) {
                        cFutureTrade.commissionAsset = trade.at("commissionAsset").as_string();
                    }
                    if (trade.has_field("time")) {
                        cFutureTrade.time = trade.at("time").as_number().to_int64();
                    }
                    if (trade.has_field("positionSide")) {
                        cFutureTrade.positionSide = trade.at("positionSide").as_string();
                    }
                    if (trade.has_field("buyer")) {
                        cFutureTrade.isBuyer = trade.at("buyer").as_bool();
                    }
                    if (trade.has_field("maker")) {
                        cFutureTrade.isMaker = trade.at("maker").as_bool();
                    }
                    
                    v.emplace_back(cFutureTrade);
                }
            }

            if (content.has_field("code")) {
                stringstream ss;
                int code = content.at("code").as_integer();
                std::string msg;
                if (content.has_field("msg")) {
                    msg = content.at("msg").as_string();
                }
                ss << "code:" << code << " msg:" << msg;
                LOG_INFO("AccountId: %d   BinanceCFuture Error: '%s' ", accountInfo.accountId, ss.str().c_str());
            }
        })
        .wait();
    } catch(exception& e) {
        LOG_INFO("BinanceCFuture Error: '%s' ", e.what());
    }

    return v;
}

bool BinanceCFuture::QueryPositionRisk(vector<binance::PositionRisk>& vPositionRisk, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vPositionRisk.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(baseUrl, config);
            http_request request(methods::GET);
            request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
            uri_builder builder(positionRiskUrl);

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

                LOG_INFO("QueryPositionRisk AccountId: %d  BinanceCFuture response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.is_array()) {
                    auto posRiskArray = content.as_array();
                    for (auto& arr : posRiskArray) {
                        binance::PositionRisk positionRisk;
                        if (arr.has_field("symbol")) {
                            positionRisk.symbol = arr.at("symbol").as_string();
                        }
                        if (arr.has_field("liquidationPrice")) {
                            positionRisk.liquidationPrice = stod(arr.at("liquidationPrice").as_string());
                        }
                        if (arr.has_field("positionSide")) {
                            positionRisk.positionSide = arr.at("positionSide").as_string();
                        }
                        vPositionRisk.emplace_back(positionRisk);
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
                    ss << "BinanceCFuture QueryPositionRisk code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryPositionRisk AccountId: %d   BinanceCFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "BinanceCFuture QueryPositionRisk code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryPositionRisk AccountId: %d   BinanceCFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
        */
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceCFuture QueryPositionRisk ") + string(e.what());
            LOG_INFO("QueryPositionRisk AccountId: %d   BinanceCFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

bool BinanceCFuture::QueryOpenOrder(vector<binance::FutureOpenOrder>& vOpenOrder, vector<string>& vErrorMsg) {
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
            uri_builder builder(openOrderUrl);

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

                LOG_INFO("QueryOpenOrder AccountId: %d  BinanceCFuture response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.is_array()) {
                    auto openOrderArray = content.as_array();
                    for (auto& arr : openOrderArray) {
                        binance::FutureOpenOrder openOrder;
                        if (arr.has_field("symbol")) {
                            openOrder.symbol = arr.at("symbol").as_string();
                        }
                        if (arr.has_field("id")) {
                            openOrder.orderId = arr.at("id").as_number().to_int64();
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
                        if (arr.has_field("time")) {
                            openOrder.time = arr.at("time").as_number().to_int64();
                        }
                        vOpenOrder.emplace_back(openOrder);
                    	LOG_INFO("BinanceCFuture AccountId: %d   QueryOpenOrder: %s", accountInfo.accountId, openOrder.toString().c_str());
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
                    ss << "BinanceCFuture QueryOpenOrder code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryOpenOrder AccountId: %d   BinanceCFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "BinanceCFuture QueryOpenOrder code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryOpenOrder AccountId: %d   BinanceCFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
        */
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceCFuture QueryOpenOrder ") + string(e.what());
            LOG_INFO("QueryOpenOrder AccountId: %d   BinanceCFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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
