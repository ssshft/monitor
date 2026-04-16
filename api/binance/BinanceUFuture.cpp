#include "binance/BinanceUFuture.h"


BinanceUFuture::BinanceUFuture(AccountInfo& info) {
    baseUrl = "https://fapi.binance.com";
    accountUrl = "/fapi/v2/account";
    tradesUrl = "/fapi/v1/userTrades";
    positionRiskUrl = "/fapi/v2/positionRisk";
    openOrderUrl = "/fapi/v1/openOrders";
    accountInfo = info;
}

BinanceUFuture::~BinanceUFuture() {
}

bool BinanceUFuture::QueryAccount(vector<binance::UFutureAsset>& vUFutureAsset, vector<binance::UFuturePosition>& vUFuturePosition, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vUFutureAsset.clear();
        vUFuturePosition.clear();
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

                LOG_INFO("QueryAccount AccountId: %d  BinanceUFuture response: '%s' ", accountInfo.accountId, response.to_string().c_str());
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
                        for (auto& asset : assetArray) {
                            binance::UFutureAsset uFutureAsset;
                            if (asset.has_field("asset")) {
                                uFutureAsset.assetType = asset.at("asset").as_string();
                            }
                            if (asset.has_field("walletBalance")) {
                                uFutureAsset.walletBalance = stod(asset.at("walletBalance").as_string());
                            }
                            if (asset.has_field("unrealizedProfit")) {
                                uFutureAsset.unrealizedProfit = stod(asset.at("unrealizedProfit").as_string());
                            }
                            if (asset.has_field("marginBalance")) {
                                uFutureAsset.marginBalance = stod(asset.at("marginBalance").as_string());
                            }
                            if (asset.has_field("maintMargin")) {
                                uFutureAsset.maintMargin = stod(asset.at("maintMargin").as_string());
                            }
                            if (asset.has_field("initialMargin")) {
                                uFutureAsset.initialMargin = stod(asset.at("initialMargin").as_string());
                            }
                            if (asset.has_field("positionInitialMargin")) {
                                uFutureAsset.positionInitialMargin = stod(asset.at("positionInitialMargin").as_string());
                            }
                            if (asset.has_field("openOrderInitialMargin")) {
                                uFutureAsset.openOrderInitialMargin = stod(asset.at("openOrderInitialMargin").as_string());
                            }
                            if (asset.has_field("crossWalletBalance")) {
                                uFutureAsset.crossWalletBalance = stod(asset.at("crossWalletBalance").as_string());
                            }
                            if (asset.has_field("crossUnPnl")) {
                                uFutureAsset.crossUnPnl = stod(asset.at("crossUnPnl").as_string());
                            }
                            if (asset.has_field("availableBalance")) {
                                uFutureAsset.availableBalance = stod(asset.at("availableBalance").as_string());
                            }
                            if (asset.has_field("maxWithdrawAmount")) {
                                uFutureAsset.maxWithdrawAmount = stod(asset.at("maxWithdrawAmount").as_string());
                            }
                            if (asset.has_field("marginAvailable")) {
                                uFutureAsset.marginAvailable = asset.at("marginAvailable").as_bool();
                            }
                            if (asset.has_field("updateTime")) {
                                uFutureAsset.updateTime = asset.at("updateTime").as_number().to_int64();
                            }
  
                            if (fabs(uFutureAsset.walletBalance) <= 0.0000000001) {
                                continue;
                            }

                            LOG_INFO("QueryAccount AccountId: %d    uFutureAsset: %s", accountInfo.accountId, uFutureAsset.toString().c_str());
                            vUFutureAsset.emplace_back(uFutureAsset);
                        }
                    }
                }

                if (content.has_field("positions")) {
                    json::value const& positions = content.at("positions");
                    if (positions.is_array()) {
                        auto positionArray = positions.as_array();
                        for (auto& position : positionArray) {
                            binance::UFuturePosition uFuturePosition;
                            if (position.has_field("symbol")) {
                                uFuturePosition.symbol = position.at("symbol").as_string();
                            } 
                            if (position.has_field("initialMargin")) {
                                uFuturePosition.initialMargin = stod(position.at("initialMargin").as_string());
                            }
                            if (position.has_field("maintMargin")) {
                                uFuturePosition.maintMargin = stod(position.at("maintMargin").as_string());
                            }
                            if (position.has_field("unrealizedProfit")) {
                                uFuturePosition.unrealizedProfit = stod(position.at("unrealizedProfit").as_string());
                            }
                            if (position.has_field("positionInitialMargin")) {
                                uFuturePosition.positionInitialMargin = stod(position.at("positionInitialMargin").as_string());
                            }
                            if (position.has_field("openOrderInitialMargin")) {
                                uFuturePosition.openOrderInitialMargin = stod(position.at("openOrderInitialMargin").as_string());
                            }
                            if (position.has_field("leverage")) {
                                uFuturePosition.leverage = stod(position.at("leverage").as_string());
                            }
                            if (position.has_field("isolated")) {
                                uFuturePosition.isolated = position.at("isolated").as_bool();
                            }
                            if (position.has_field("entryPrice")) {
                                uFuturePosition.entryPrice = stod(position.at("entryPrice").as_string());
                            }
                            if (position.has_field("maxNotional")) {
                                uFuturePosition.maxNotional = stod(position.at("maxNotional").as_string());
                            }
                            if (position.has_field("bidNotional")) {
                                uFuturePosition.bidNotional = stod(position.at("bidNotional").as_string());
                            }
                            if (position.has_field("askNotional")) {
                                uFuturePosition.askNotional = stod(position.at("askNotional").as_string());
                            }
                            if (position.has_field("positionSide")) {
                                uFuturePosition.positionSide = position.at("positionSide").as_string();
                            }
                            if (position.has_field("positionAmt")) {
                                /*
                                if (uFuturePosition.positionSide == "SHORT") {
                                    uFuturePosition.positionAmt = -stod(position.at("positionAmt").as_string());
                                } else {
                                    uFuturePosition.positionAmt = stod(position.at("positionAmt").as_string());
                                }
                                */
                                uFuturePosition.positionAmt = stod(position.at("positionAmt").as_string());
                            }
                            if (position.has_field("updateTime")) {
                                uFuturePosition.updateTime = position.at("updateTime").as_number().to_int64();
                            }
                        
                            if (fabs(uFuturePosition.positionAmt) <= 0.0000000001) {
                                continue;
                            }

                            LOG_INFO("QueryAccount AccountId: %d    uFuturePosition: %s", accountInfo.accountId, uFuturePosition.toString().c_str());
                            vUFuturePosition.emplace_back(uFuturePosition);
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
                    ss << "BinanceUFuture QueryAccount code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   BinanceUFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "BinanceUFuture QueryAccount code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryAccount AccountId: %d   BinanceUFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
        */
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceUFuture QueryAccount ") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   BinanceUFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

vector<binance::UFutureTrade> BinanceUFuture::QueryTrades(string symbol, int days) {
    vector<binance::UFutureTrade> v;
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

        request.set_request_uri(builder.to_string());
        client.request(request)
        .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
            auto code = response.status_code();
            if(code == status_codes::OK || code == status_codes::BadRequest) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                return response.extract_json();
            }
                
            LOG_INFO("BinanceUFuture response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();
            if (content.is_array()) {
                auto tradeArray = content.as_array();
                for(auto& trade : tradeArray) {
                    binance::UFutureTrade uFutureTrade;
                    if (trade.has_field("symbol")) {
                        uFutureTrade.symbol = trade.at("symbol").as_string();
                    }
                    if (trade.has_field("id")) {
                        uFutureTrade.id = trade.at("id").as_number().to_int64();
                    }
                    if (trade.has_field("orderId")) {
                        uFutureTrade.orderId = trade.at("orderId").as_number().to_int64();
                    }
                    if (trade.has_field("price")) {
                        uFutureTrade.price = stod(trade.at("price").as_string());
                    }
                    if (trade.has_field("qty")) {
                        uFutureTrade.qty = stod(trade.at("qty").as_string());
                    }
                    if (trade.has_field("quoteQty")) {
                        uFutureTrade.quoteQty = stod(trade.at("quoteQty").as_string());
                    }
                    if (trade.has_field("realizedPnl")) {
                        uFutureTrade.realizedPnl = stod(trade.at("realizedPnl").as_string());
                    }
                    if (trade.has_field("side")) {
                        uFutureTrade.side = trade.at("side").as_string();
                    }
                    if (trade.has_field("positionSide")) {
                        uFutureTrade.positionSide = trade.at("positionSide").as_string();
                    }
                    if (trade.has_field("commission")) {
                        uFutureTrade.commission = stod(trade.at("commission").as_string());
                    }
                    if (trade.has_field("commissionAsset")) {
                        uFutureTrade.commissionAsset = trade.at("commissionAsset").as_string();
                    }
                    if (trade.has_field("time")) {
                        uFutureTrade.time = trade.at("time").as_number().to_int64();
                    }
                    if (trade.has_field("buyer")) {
                        uFutureTrade.isBuyer = trade.at("buyer").as_bool();
                    }
                    if (trade.has_field("maker")) {
                        uFutureTrade.isMaker = trade.at("maker").as_bool();
                    }
                   
                    v.emplace_back(uFutureTrade);
                }
            }
        })
        .wait();
    } catch(exception& e) {
        LOG_INFO("BinanceUFuture Error: '%s' ", e.what());
    }

    return v;
}

bool BinanceUFuture::QueryPositionRisk(vector<binance::PositionRisk>& vPositionRisk, vector<string>& vErrorMsg) {
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

                LOG_INFO("QueryPositionRisk AccountId: %d  BinanceUFuture response: '%s' ", accountInfo.accountId, response.to_string().c_str());
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
                    ss << "BinanceUFuture QueryPositionRisk code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryPositionRisk AccountId: %d   BinanceUFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }

            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "BinanceUFuture QueryPositionRisk code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryPositionRisk AccountId: %d   BinanceUFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
        */
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceUFuture QueryPositionRisk ") + string(e.what());
            LOG_INFO("QueryPositionRisk AccountId: %d   BinanceUFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

bool BinanceUFuture::QueryOpenOrder(vector<binance::FutureOpenOrder>& vOpenOrder, vector<string>& vErrorMsg) {
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

                LOG_INFO("QueryOpenOrder AccountId: %d  BinanceUFuture response: '%s' ", accountInfo.accountId, response.to_string().c_str());
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
                    	LOG_INFO("BinanceUFuture AccountId: %d  QueryOpenOrder: %s", accountInfo.accountId, openOrder.toString().c_str());
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
                    ss << "BinanceUFuture QueryOpenOrder code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryOpenOrder AccountId: %d   BinanceUFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "BinanceUFuture QueryOpenOrder code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryOpenOrder AccountId: %d   BinanceUFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
        */
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceUFuture QueryOpenOrder ") + string(e.what());
            LOG_INFO("QueryOpenOrder AccountId: %d   BinanceUFuture Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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
