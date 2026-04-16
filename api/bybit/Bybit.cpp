#include "bybit/Bybit.h"


Bybit::Bybit(AccountInfo& info) {
    accountUrl = "/v5/account/wallet-balance";
    positionUrl = "/v5/position/list";
    orderUrl = "/v5/order/realtime";
    accountInfo = info;
}

Bybit::~Bybit() {
}

bool Bybit::QueryAccount(bybit::TotalAccountInfo& totalAccountInfo, vector<bybit::Asset>& vAsset, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vAsset.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            uri_builder builder(accountUrl);
            
            int64_t timestamp = gettickcount(); // ms
            int recvWindow = 5000;
            stringstream ss;
            //ss << timestamp << accountInfo.apiKey << recvWindow << "accountType=UNIFIED";
            ss << timestamp << accountInfo.apiKey << recvWindow << "accountType=CONTRACT";
            string message = ss.str();
            string signature = getSignature(message, accountInfo.secretKey);

            request.headers().add("Accept", "application/json");  
            request.headers().add("Content-Type", "application/json");  
            request.headers().add("X-BAPI-SIGN", signature);  
            request.headers().add("X-BAPI-TIMESTAMP", timestamp);
            request.headers().add("X-BAPI-API-KEY", accountInfo.apiKey);
            request.headers().add("X-BAPI-RECV-WINDOW", recvWindow);
       
            //builder.append_query("accountType", "UNIFIED");
            builder.append_query("accountType", "CONTRACT");
            request.set_request_uri(builder.to_string());
            
            client.request(request)
            .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if(code == status_codes::OK || code == status_codes::BadRequest || code == status_codes::NotFound) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                    return response.extract_json();
                }
                
                LOG_INFO("Bybit QueryAccount response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.has_field("result")) {
                    auto& res = content.at("result");
                    if (res.has_field("list")) {
                        auto& li = res.at("list");
                        if (li.is_array()) {
                            auto& liArray = li.as_array();
                            for (auto& arr: liArray) {
                                //total info
                                if (arr.has_field("accountType")) {
                                    totalAccountInfo.accountType = arr.at("accountType").as_string();
                                }

                                if (arr.has_field("totalEquity")) {
                                    string s = arr.at("totalEquity").as_string();
                                    if (s != "") {
                                        totalAccountInfo.totalEquity = stod(s);
                                    }
                                }

                                if (arr.has_field("accountIMRate")) {
                                    string s = arr.at("accountIMRate").as_string();
                                    if (s != "") {
                                        totalAccountInfo.accountIMRate = stod(s);
                                    }
                                }

                                if (arr.has_field("totalMarginBalance")) {
                                    string s = arr.at("totalMarginBalance").as_string();
                                    if (s != "") {
                                        totalAccountInfo.totalMarginBalance = stod(s);
                                    }
                                }

                                if (arr.has_field("totalInitialMargin")) {
                                    string s = arr.at("totalInitialMargin").as_string();
                                    if (s != "") {
                                        totalAccountInfo.totalInitialMargin = stod(s);
                                    }
                                }

                                if (arr.has_field("totalAvailableBalance")) {
                                    string s = arr.at("totalAvailableBalance").as_string();
                                    if (s != "") {
                                        totalAccountInfo.totalAvailableBalance = stod(s);
                                    }
                                }

                                if (arr.has_field("accountMMRate")) {
                                    string s = arr.at("accountMMRate").as_string();
                                    if (s != "") {
                                        totalAccountInfo.accountMMRate = stod(s);
                                    }
                                }

                                if (arr.has_field("totalPerpUPL")) {
                                    string s = arr.at("totalPerpUPL").as_string();
                                    if (s != "") {
                                        totalAccountInfo.totalPerpUPL = stod(s);
                                    }
                                }

                                if (arr.has_field("totalWalletBalance")) {
                                    string s = arr.at("totalWalletBalance").as_string();
                                    if (s != "") {
                                        totalAccountInfo.totalWalletBalance = stod(s);
                                    }
                                }

                                if (arr.has_field("accountLTV")) {
                                    string s = arr.at("accountLTV").as_string();
                                    if (s != "") {
                                        totalAccountInfo.accountLTV = stod(s);
                                    }
                                }

                                if (arr.has_field("totalMaintenanceMargin")) {
                                    string s = arr.at("totalMaintenanceMargin").as_string();
                                    if (s != "") {
                                        totalAccountInfo.totalMaintenanceMargin = stod(s);
                                    }
                                }

                                LOG_INFO("QueryAccount AccountId: %d   Bybit totalAccountInfo: %s", accountInfo.accountId, totalAccountInfo.toString().c_str());

                                // coin info
                                auto& coinCon = arr.at("coin");
                                if (coinCon.is_array()) {
                                    auto& coinArray = coinCon.as_array();
                                    for (auto& coin: coinArray) {
                                        bybit::Asset asset;
                                        if (coin.has_field("coin")) {
                                            asset.coin = coin.at("coin").as_string();
                                        }

                                        if (coin.has_field("availableToBorrow")) {
                                            string s = coin.at("availableToBorrow").as_string();
                                            if (s != "") {
                                                asset.availableToBorrow = stod(s);
                                            }
                                        }

                                        if (coin.has_field("bonus")) {
                                            string s = coin.at("bonus").as_string();
                                            if (s != "") {
                                                asset.bonus = stod(s);
                                            }
                                        }

                                        if (coin.has_field("accruedInterest")) {
                                            string s = coin.at("accruedInterest").as_string();
                                            if (s != "") {
                                                asset.accruedInterest = stod(s);
                                            }
                                        }

                                        if (coin.has_field("availableToWithdraw")) {
                                            string s = coin.at("availableToWithdraw").as_string();
                                            if (s != "") {
                                                asset.availableToWithdraw = stod(s);
                                            }
                                        }

                                        if (coin.has_field("totalOrderIM")) {
                                            string s = coin.at("totalOrderIM").as_string();
                                            if (s != "") {
                                                asset.totalOrderIM = stod(s);
                                            }
                                        }

                                        if (coin.has_field("equity")) {
                                            string s = coin.at("equity").as_string();
                                            if (s != "") {
                                                asset.equity = stod(s);
                                            }
                                        }

                                        if (coin.has_field("totalPositionMM")) {
                                            string s = coin.at("totalPositionMM").as_string();
                                            if (s != "") {
                                                asset.totalPositionMM = stod(s);
                                            }
                                        }

                                        if (coin.has_field("usdValue")) {
                                            string s = coin.at("usdValue").as_string();
                                            if (s != "") {
                                                asset.usdValue = stod(s);
                                            }
                                        }

                                        if (coin.has_field("unrealisedPnl")) {
                                            string s = coin.at("unrealisedPnl").as_string();
                                            if (s != "") {
                                                asset.unrealisedPnl = stod(s);
                                            }
                                        }

                                        if (coin.has_field("borrowAmount")) {
                                            string s = coin.at("borrowAmount").as_string();
                                            if (s != "") {
                                                asset.borrowAmount = stod(s);
                                            }
                                        }

                                        if (coin.has_field("equity")) {
                                            string s = coin.at("equity").as_string();
                                            if (s != "") {
                                                asset.equity = stod(s);
                                            }
                                        }

                                        if (coin.has_field("totalPositionIM")) {
                                            string s = coin.at("totalPositionIM").as_string();
                                            if (s != "") {
                                                asset.totalPositionIM = stod(s);
                                            }
                                        }

                                        if (coin.has_field("walletBalance")) {
                                            string s = coin.at("walletBalance").as_string();
                                            if (s != "") {
                                                asset.walletBalance = stod(s);
                                            }
                                        }

                                        if (coin.has_field("cumRealisedPnl")) {
                                            string s = coin.at("cumRealisedPnl").as_string();
                                            if (s != "") {
                                                asset.cumRealisedPnl = stod(s);
                                            }
                                        }

                                        if (fabs(asset.walletBalance) >= 0.0000000001) {
                                            LOG_INFO("QueryAccount AccountId: %d   Bybit asset: %s", accountInfo.accountId, asset.toString().c_str());
                                            vAsset.emplace_back(asset);       
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
              
                if (content.has_field("retCode")) {
                    int retCode = content.at("retCode").as_integer();
                    if (retCode != 0) {
                        query = false;
                        stringstream ss;
                        string msg = "";
                        if (content.has_field("retMsg")) {
                            msg = content.at("retMsg").as_string();
                        }
                        ss << "Bybit QueryAccount msg:" << msg;
                        string errMsg = ss.str();
                        LOG_INFO("QueryAccount AccountId: %d   Bybit Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                        vErrorMsg.emplace_back(errMsg);
                    }
                }      

            })
            .wait();
        } catch(exception& e) {
            query = false;
            string errMsg = string("Bybit QueryAccount") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   Bybit Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

bool Bybit::QueryPosition(vector<bybit::Position>& vPosition, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vPosition.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            uri_builder builder(positionUrl);
            
            int64_t timestamp = gettickcount(); // ms
            int recvWindow = 5000;
            stringstream ss;
            ss << timestamp << accountInfo.apiKey << recvWindow << "category=linear&settleCoin=USDT&limit=200";
            string message = ss.str();
            string signature = getSignature(message, accountInfo.secretKey);
       
            request.headers().add("Accept", "application/json");  
            request.headers().add("Content-Type", "application/json");  
            request.headers().add("X-BAPI-SIGN", signature);  
            request.headers().add("X-BAPI-TIMESTAMP", timestamp);
            request.headers().add("X-BAPI-API-KEY", accountInfo.apiKey);
            request.headers().add("X-BAPI-RECV-WINDOW", recvWindow);
       
            builder.append_query("category", "linear");
            builder.append_query("settleCoin", "USDT");
            builder.append_query("limit", 200);
            request.set_request_uri(builder.to_string());
            
            client.request(request)
            .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if(code == status_codes::OK || code == status_codes::BadRequest || code == status_codes::NotFound) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                    return response.extract_json();
                }
                
                LOG_INFO("Bybit QueryPosition response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.has_field("result")) {
                    auto& res = content.at("result");
                    if (res.has_field("list")) {
                        auto& li = res.at("list");
                        if (li.is_array()) {
                            auto& liArray = li.as_array();
                            for (auto& arr: liArray) {
                                bybit::Position position;
                                if (arr.has_field("symbol")) {
                                    position.symbol = arr.at("symbol").as_string();
                                }

                                if (arr.has_field("size")) {
                                    string s = arr.at("size").as_string();
                                    if (s != "") {
                                        position.size = stod(s);
                                    }
                                }

                                if (arr.has_field("side")) {
                                    position.side = arr.at("side").as_string();
                                }

                                if (arr.has_field("leverage")) {
                                    string s = arr.at("leverage").as_string();
                                    if (s != "") {
                                        position.leverage = stod(s);
                                    } 
                                }

                                if (arr.has_field("avgPrice")) {
                                    string s = arr.at("avgPrice").as_string();
                                    if (s != "") {
                                        position.avgPrice = stod(s);
                                    }  
                                }

                                if (arr.has_field("liqPrice")) {
                                    string s = arr.at("liqPrice").as_string();
                                    if (s != "") {
                                        position.liqPrice = stod(s);
                                    } 
                                }

                                if (arr.has_field("takeProfit")) {
                                    string s = arr.at("takeProfit").as_string();
                                    if (s != "") {
                                        position.takeProfit = stod(s);
                                    }
                                }

                                if (arr.has_field("unrealisedPnl")) {
                                    string s = arr.at("unrealisedPnl").as_string();
                                    if (s != "") {
                                        position.unrealisedPnl = stod(s);
                                    }
                                }

                                if (arr.has_field("markPrice")) {
                                    string s = arr.at("markPrice").as_string();
                                    if (s != "") {
                                        position.markPrice = stod(s);
                                    } 
                                }

                                if (arr.has_field("cumRealisedPnl")) {
                                    string s = arr.at("cumRealisedPnl").as_string();
                                    if (s != "") {
                                        position.cumRealisedPnl = stod(s);
                                    } 
                                }

                                if (arr.has_field("positionMM")) {
                                    string s = arr.at("positionMM").as_string();
                                    if (s != "") {
                                        position.positionMM = stod(s);
                                    } 
                                }

                                if (fabs(position.size) > 0.0000000001) {
            		                LOG_INFO("QueryPosition AccountId: %d   Bybit position: %s", accountInfo.accountId, position.toString().c_str());
                                    if (position.side == "Buy") {
                                        position.size = fabs(position.size);
                                    } else {
                                        position.size = -fabs(position.size);
                                    }
                                    vPosition.emplace_back(position);
                                }
                            }
                        }
                    }
                }
                
              
                if (content.has_field("retCode")) {
                    int retCode = content.at("retCode").as_integer();
                    if (retCode != 0) {
                        query = false;
                        stringstream ss;
                        string msg = "";
                        if (content.has_field("retMsg")) {
                            msg = content.at("retMsg").as_string();
                        }
                        ss << "Bybit QueryPosition msg:" << msg;
                        string errMsg = ss.str();
                        LOG_INFO("QueryPosition AccountId: %d   Bybit Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                        vErrorMsg.emplace_back(errMsg);
                    }
                }      

            })
            .wait();
        } catch(exception& e) {
            query = false;
            string errMsg = string("Bybit QueryPosition ") + string(e.what());
            LOG_INFO("QueryPosition AccountId: %d   Bybit Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

bool Bybit::QueryOpenOrder(vector<bybit::Order>& vOpenOrder, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vOpenOrder.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            uri_builder builder(orderUrl);
            
            int64_t timestamp = gettickcount(); // ms
            int recvWindow = 5000;
            stringstream ss;
            ss << timestamp << accountInfo.apiKey << recvWindow << "category=linear&settleCoin=USDT&orderFilter=Order&limit=50";
            string message = ss.str();
            string signature = getSignature(message, accountInfo.secretKey);
       
            request.headers().add("Accept", "application/json");  
            request.headers().add("Content-Type", "application/json");  
            request.headers().add("X-BAPI-SIGN", signature);  
            request.headers().add("X-BAPI-TIMESTAMP", timestamp);
            request.headers().add("X-BAPI-API-KEY", accountInfo.apiKey);
            request.headers().add("X-BAPI-RECV-WINDOW", recvWindow);
       
            builder.append_query("category", "linear");
            builder.append_query("settleCoin", "USDT");
            builder.append_query("orderFilter", "Order");
            builder.append_query("limit", 50); 
            request.set_request_uri(builder.to_string());
            
            client.request(request)
            .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if(code == status_codes::OK || code == status_codes::BadRequest || code == status_codes::NotFound) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                    return response.extract_json();
                }
                
                LOG_INFO("Bybit QueryOpenOrder response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.has_field("result")) {
                    auto& res = content.at("result");
                    if (res.has_field("list")) {
                        auto& li = res.at("list");
                        if (li.is_array()) {
                            auto& liArray = li.as_array();
                            for (auto& arr: liArray) {
                                bybit::Order order;
                                if (arr.has_field("orderId")) {
                                    order.orderId = arr.at("orderId").as_string();
                                }

                                if (arr.has_field("orderLinkId")) {
                                    order.orderLinkId = arr.at("orderLinkId").as_string();
                                }

                                if (arr.has_field("symbol")) {
                                    order.symbol = arr.at("symbol").as_string();
                                }

                                if (arr.has_field("price")) {
                                    order.price = stod(arr.at("price").as_string());
                                }

                                if (arr.has_field("qty")) {
                                    order.qty = stod(arr.at("qty").as_string());
                                }

                                if (arr.has_field("side")) {
                                    order.side = arr.at("side").as_string();
                                }

                                if (arr.has_field("avgPrice")) {
                                    order.avgPrice = stod(arr.at("avgPrice").as_string());
                                }

                                if (arr.has_field("cumExecQty")) {
                                    order.cumExecQty = stod(arr.at("cumExecQty").as_string());
                                }

                                if (arr.has_field("cumExecValue")) {
                                    order.cumExecValue = stod(arr.at("cumExecValue").as_string());
                                }

                                if (arr.has_field("cumExecFee")) {
                                    order.cumExecFee = stod(arr.at("cumExecFee").as_string());
                                }

                                if (arr.has_field("timeInForce")) {
                                    order.timeInForce = arr.at("timeInForce").as_string();
                                }

                                if (arr.has_field("orderType")) {
                                    order.orderType = arr.at("orderType").as_string();
                                }

                                if (arr.has_field("createdTime")) {
                                    order.createdTime = stoll(arr.at("createdTime").as_string());
                                }

                                if (arr.has_field("updatedTime")) {
                                    order.updatedTime = stoll(arr.at("updatedTime").as_string());
                                }

                                if (fabs(order.qty) > 0.0000000001) {
            		                LOG_INFO("QueryOpenOrder AccountId: %d   Bybit Order: %s", accountInfo.accountId, order.toString().c_str());
                                    vOpenOrder.emplace_back(order);
                                }
                            }
                        }
                    }
                }
                
              
                if (content.has_field("retCode")) {
                    int retCode = content.at("retCode").as_integer();
                    if (retCode != 0) {
                        query = false;
                        stringstream ss;
                        string msg = "";
                        if (content.has_field("retMsg")) {
                            msg = content.at("retMsg").as_string();
                        }
                        ss << "Bybit QueryOpenOrder msg:" << msg;
                        string errMsg = ss.str();
                        LOG_INFO("QueryOpenOrder AccountId: %d   Bybit Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                        vErrorMsg.emplace_back(errMsg);
                    }
                }      

            })
            .wait();
        } catch(exception& e) {
            query = false;
            string errMsg = string("Bybit QueryOpenOrder") + string(e.what());
            LOG_INFO("QueryOpenOrder AccountId: %d   Bybit Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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