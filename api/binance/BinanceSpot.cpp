#include "binance/BinanceSpot.h"


BinanceSpot::BinanceSpot(AccountInfo& info) {
    accountUrl = "/api/v3/account";
    tradesUrl = "/api/v3/myTrades";
    openOrderUrl = "/api/v3/openOrders";
    accountInfo = info;
}

BinanceSpot::~BinanceSpot() {
}

bool BinanceSpot::QueryAccount(vector<binance::SpotAsset>& vSpotAsset, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vSpotAsset.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
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

                LOG_INFO("QueryAccount AccountId: %d  BinanceSpot response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.has_field("balances")) {
                    json::value const& balances = content.at("balances");
                    if (balances.is_array()) {
                        auto assetArray = balances.as_array();
                        for (auto& asset : assetArray) {
                            binance::SpotAsset spotAsset;
                            if (asset.has_field("asset")) {
                                spotAsset.assetType = asset.at("asset").as_string();
                            }
                            if (asset.has_field("free")) {
                                spotAsset.free = stod(asset.at("free").as_string());
                            }
                            if (asset.has_field("locked")) {
                                spotAsset.locked = stod(asset.at("locked").as_string());
                            }
                        
                            if (fabs(spotAsset.free + spotAsset.locked) <= 0.0000000001) {
                                continue;
                            }

                            LOG_INFO("QueryAccount AccountId: %d    spotAsset: %s", accountInfo.accountId, spotAsset.toString().c_str());
                            vSpotAsset.emplace_back(spotAsset);
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
                    ss << "BinanceSpot QueryAccount code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   BinanceSpot Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }

            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "BinanceSpot QueryAccount code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryAccount AccountId: %d   BinanceSpot Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
        */
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceSpot QueryAccount ") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   BinanceSpot Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

vector<binance::SpotTrade> BinanceSpot::QueryTrades(string symbol, int days) {
    vector<binance::SpotTrade> v;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(accountInfo.restUrl, config);
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
                
            LOG_INFO("BinanceSpot response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();
            if (content.is_array()) {
                auto tradeArray = content.as_array();
                for(auto& trade : tradeArray) {
                    binance::SpotTrade spotTrade;
                    if (trade.has_field("symbol")) {
                        spotTrade.symbol = trade.at("symbol").as_string();
                    }
                    if (trade.has_field("id")) {
                        spotTrade.id = trade.at("id").as_number().to_int64();
                    }
                    if (trade.has_field("orderId")) {
                        spotTrade.orderId = trade.at("orderId").as_number().to_int64();
                    }
                    if (trade.has_field("orderListId")) {
                        spotTrade.orderListId = trade.at("orderListId").as_number().to_int64();
                    }
                    if (trade.has_field("price")) {
                        spotTrade.price = stod(trade.at("price").as_string());
                    }
                    if (trade.has_field("qty")) {
                        spotTrade.qty = stod(trade.at("qty").as_string());
                    }
                    if (trade.has_field("quoteQty")) {
                        spotTrade.quoteQty = stod(trade.at("quoteQty").as_string());
                    }
                    if (trade.has_field("commission")) {
                        spotTrade.commission = stod(trade.at("commission").as_string());
                    }
                    if (trade.has_field("commissionAsset")) {
                        spotTrade.commissionAsset = trade.at("commissionAsset").as_string();
                    }
                    if (trade.has_field("time")) {
                        spotTrade.time = trade.at("time").as_number().to_int64();
                    }
                    if (trade.has_field("isBuyer")) {
                        spotTrade.isBuyer = trade.at("isBuyer").as_bool();
                    }
                    if (trade.has_field("isMaker")) {
                        spotTrade.isMaker = trade.at("isMaker").as_bool();
                    }
                    if (trade.has_field("isBestMatch")) {
                        spotTrade.isBestMatch = trade.at("isBestMatch").as_bool();
                    }
                    
                    v.emplace_back(spotTrade);
                }
            }
        })
        .wait();
    } catch(exception& e) {
        LOG_INFO("BinanceSpot Error: '%s' ", e.what());
    }

    return v;
}

vector<binance::SpotOpenOrder> BinanceSpot::QueryOpenOrder() {
    vector<binance::SpotOpenOrder> v;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(accountInfo.restUrl, config);
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
        .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
            auto code = response.status_code();
            if(code == status_codes::OK || code == status_codes::BadRequest) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                return response.extract_json();
            }
                
            LOG_INFO("BinanceSpot response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();
            if (content.is_array()) {
                auto openOrderArray = content.as_array();
                for(auto& arr : openOrderArray) {
                    binance::SpotOpenOrder spotOpenOrder;
                    if (arr.has_field("symbol")) {
                        spotOpenOrder.symbol = arr.at("symbol").as_string();
                    }
                    if (arr.has_field("orderId")) {
                        spotOpenOrder.orderId = arr.at("orderId").as_number().to_int64();
                    }
                    if (arr.has_field("clientOrderId")) {
                        spotOpenOrder.clientOrderId = arr.at("clientOrderId").as_string();
                    }
                    if (arr.has_field("price")) {
                        spotOpenOrder.price = stod(arr.at("price").as_string());
                    }
                    if (arr.has_field("origQty")) {
                        spotOpenOrder.origQty = stod(arr.at("origQty").as_string());
                    }
                    if (arr.has_field("executedQty")) {
                        spotOpenOrder.executedQty = stod(arr.at("executedQty").as_string());
                    }
                    if (arr.has_field("cummulativeQuoteQty")) {
                        spotOpenOrder.cumQuote = stod(arr.at("cummulativeQuoteQty").as_string());
                    }
                    if (arr.has_field("side")) {
                        spotOpenOrder.side = arr.at("side").as_string();
                    }
                    if (arr.has_field("time")) {
                        spotOpenOrder.time = arr.at("time").as_number().to_int64();
                    }
                    
                    //LOG_INFO("spot accountId: %d  openOrder: %s", accountInfo.accountId, spotOpenOrder.toString().c_str());
                    v.emplace_back(spotOpenOrder);
                }
            }
        })
        .wait();
    } catch(exception& e) {
        LOG_INFO("BinanceSpot Error: '%s' ", e.what());
    }

    return v;
}
