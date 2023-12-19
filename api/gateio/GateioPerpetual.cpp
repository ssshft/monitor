#include "GateioPerpetual.h"


GateioPerpetual::GateioPerpetual(AccountInfo& info) {
    accountUrl = "/api/v4/futures/";
    positionUrl = "/api/v4/futures/";
    orderUrl = "/api/v4/futures/";
    accountInfo = info;
}

GateioPerpetual::~GateioPerpetual() {
}

bool GateioPerpetual::QueryAccount(vector<gateio::FutureAsset>& vFutureAsset, vector<string>& vErrorMsg) {
    bool query = true;
    vector<string> vSettle;
    if (accountInfo.unified == 1) {
        vSettle.push_back("usdt");
    } else {
        vSettle.push_back("usdt");
        vSettle.push_back("btc");
    }
    for (size_t i = 0; i < vSettle.size(); ++i) {
        vector<gateio::FutureAsset> vSettleAccount;
        vector<string> vErr;
        bool querySettleAccount = QuerySettleAccount(vSettle[i], vSettleAccount, vErr);
        query = query && querySettleAccount;
        vFutureAsset.insert(vFutureAsset.end(), vSettleAccount.begin(), vSettleAccount.end());
        vErrorMsg.insert(vErrorMsg.end(), vErr.begin(), vErr.end());
    }
    return query;
}

bool GateioPerpetual::QueryPosition(vector<gateio::FuturePosition>& vFuturePosition, vector<string>& vErrorMsg) {
    bool query = true;
    vector<string> vSettle;
    if (accountInfo.unified == 1) {
        vSettle.push_back("usdt");
    } else {
        vSettle.push_back("usdt");
        vSettle.push_back("btc");
    }
    for (size_t i = 0; i < vSettle.size(); ++i) {
        vector<gateio::FuturePosition> vSettlePosition;
        vector<string> vErr;
        bool querySettlePosition = QuerySettlePosition(vSettle[i], vSettlePosition, vErr);
        query = query && querySettlePosition;
        vFuturePosition.insert(vFuturePosition.end(), vSettlePosition.begin(), vSettlePosition.end());
        vErrorMsg.insert(vErrorMsg.end(), vErr.begin(), vErr.end());
    }
    return query;
}

bool GateioPerpetual::QueryOpenOrder(vector<gateio::FutureOrder>& vFutureOrder, vector<string>& vErrorMsg) {
    bool query = true;
    vector<string> vSettle;
    if (accountInfo.unified == 1) {
        vSettle.push_back("usdt");
    } else {
        vSettle.push_back("usdt");
        vSettle.push_back("btc");
    }
    for (size_t i = 0; i < vSettle.size(); ++i) {
        vector<gateio::FutureOrder> vSettleOrder;
        vector<string> vErr;
        bool querySettleOpenOrder = QuerySettleOpenOrder(vSettle[i], vSettleOrder, vErr);
        query = query && querySettleOpenOrder;
        vFutureOrder.insert(vFutureOrder.end(), vSettleOrder.begin(), vSettleOrder.end());
        vErrorMsg.insert(vErrorMsg.end(), vErr.begin(), vErr.end());
    }
    return query;
}

bool GateioPerpetual::QueryOrder(vector<gateio::FutureOrder>& vFutureOrder, vector<string>& vErrorMsg) {
    bool query = true;
    vector<string> vSettle;
    if (accountInfo.unified == 1) {
        vSettle.push_back("usdt");
    } else {
        vSettle.push_back("usdt");
        vSettle.push_back("btc");
    }
    for (size_t i = 0; i < vSettle.size(); ++i) {
        vector<gateio::FutureOrder> vSettleOrder;
        vector<string> vErr;
        bool querySettleOpenOrder = QuerySettleOrder(vSettle[i], vSettleOrder, vErr);
        query = query && querySettleOpenOrder;
        vFutureOrder.insert(vFutureOrder.end(), vSettleOrder.begin(), vSettleOrder.end());
        vErrorMsg.insert(vErrorMsg.end(), vErr.begin(), vErr.end());
    }
    return query;
}

bool GateioPerpetual::QuerySettleAccount(string settle, vector<gateio::FutureAsset>& vFutureAsset, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vFutureAsset.clear();
        vErrorMsg.clear();

        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            string accountSettleUrl = accountUrl + settle + "/accounts";
            uri_builder builder(accountSettleUrl);

            string hashStr = sha512("");
            int64_t timestamp = gettickcount() / 1000;
            stringstream ss;
            ss << "GET" << "\n" << accountSettleUrl << "\n" << "" << "\n" << hashStr << "\n" << timestamp;
            string message = ss.str();
            string signature = getSignatureGate(message, accountInfo.secretKey);
            request.headers().add("Accept", "application/json");  
            request.headers().add("Content-Type", "application/json");  
            request.headers().add("SIGN", signature);  
            request.headers().add("Timestamp", timestamp);
            request.headers().add("KEY", accountInfo.apiKey);

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if (code == status_codes::OK || code == status_codes::BadRequest || code == status_codes::NotFound) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                    return response.extract_json();
                }
                
                LOG_INFO("GateioPerpetual QueryAccount response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                //LOG_INFO("GateioPerpetual QueryAccount AccountId: %d  content: %s", accountInfo.accountId, content.to_string().c_str());
                gateio::FutureAsset futureAsset;

                if (content.has_field("currency")) {
                    string currency = content.at("currency").as_string();
                    futureAsset.currency = boost::to_upper_copy(currency);
                }
                if (content.has_field("total")) {
                    futureAsset.total = fabs(stod(content.at("total").as_string()));
                }
                if (content.has_field("unrealised_pnl")) {
                    futureAsset.unrealisedPnl = stod(content.at("unrealised_pnl").as_string());
                }
                if (content.has_field("position_margin")) {
                    futureAsset.positionMargin = stod(content.at("position_margin").as_string());
                }
                if (content.has_field("order_margin")) {
                    futureAsset.orderMargin = stod(content.at("order_margin").as_string());
                }
                if (content.has_field("available")) {
                    futureAsset.available = fabs(stod(content.at("available").as_string()));
                }
                if (content.has_field("position_initial_margin")) {
                    futureAsset.positionInitialMargin = stod(content.at("position_initial_margin").as_string());
                }
                if (content.has_field("point")) {
                    futureAsset.point = stod(content.at("point").as_string());
                }
                if (content.has_field("bonus")) {
                    futureAsset.bonus = stod(content.at("bonus").as_string());
                }
                if (content.has_field("in_dual_mode")) {
                    futureAsset.inDualMode = content.at("in_dual_mode").as_bool();
                }

                if (futureAsset.total >= 0.0000000001) {
            	    LOG_INFO("QueryAccount AccountId: %d   GateioPerpetual asset: %s", accountInfo.accountId, futureAsset.toString().c_str());
                    vFutureAsset.emplace_back(futureAsset);
                }

	        if (content.has_field("label")) {
                    string label = content.at("label").as_string();
		    if (label != "USER_NOT_FOUND") {
                    	query = false;
                    	stringstream ss;
                    	string msg = "";
                    	if (label.size() > 0) {
                            if (content.has_field("message")) {
                                msg = content.at("message").as_string();
                            } else {
                                msg = content.at("label").as_string();
                            }
                        }
                        ss << "GateioPerpetual QueryAccount msg:" << msg;
                        string errMsg = ss.str();
                        LOG_INFO("QueryAccount AccountId: %d   GateioPerpetual Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                        vErrorMsg.emplace_back(errMsg);
		    }
                }

            })
            .wait();
        } catch(exception& e) {
            query = false;
            string errMsg = string("GateioPerpetual QueryAccount ") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d  settle:%s GateioPerpetual Error: '%s' ", accountInfo.accountId, settle.c_str(), errMsg.c_str());
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

bool GateioPerpetual::QuerySettlePosition(string settle, vector<gateio::FuturePosition>& vFuturePosition, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vFuturePosition.clear();
        vErrorMsg.clear();

        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            string positionSettleUrl = positionUrl + settle + "/positions";
            uri_builder builder(positionSettleUrl);

            string hashStr = sha512("");
            int64_t timestamp = gettickcount() / 1000;
            stringstream ss;
            ss << "GET" << "\n" << positionSettleUrl << "\n" << "" << "\n" << hashStr << "\n" << timestamp;
            string message = ss.str();
            string signature = getSignatureGate(message, accountInfo.secretKey);
            request.headers().add("Accept", "application/json");  
            request.headers().add("Content-Type", "application/json");  
            request.headers().add("SIGN", signature);  
            request.headers().add("Timestamp", timestamp);
            request.headers().add("KEY", accountInfo.apiKey);

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if (code == status_codes::OK || code == status_codes::BadRequest || code == status_codes::NotFound) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                    return response.extract_json();
                }
                
                LOG_INFO("GateioPerpetual QueryPosition response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                //LOG_INFO("GateioPerpetual QuerySettlePosition AccountId: %d  content: %s", accountInfo.accountId, content.to_string().c_str());
                if (content.is_array()) {
                    auto positionArray = content.as_array();
                    for (auto& position: positionArray) {
                        gateio::FuturePosition futurePosition;
                        if (position.has_field("contract")) {
                            futurePosition.contract = position.at("contract").as_string();
                        }
                        futurePosition.instrumentType = settle;
                        if (position.has_field("size")) {
                            futurePosition.size = position.at("size").as_number().to_int64();
                        }
                        if (position.has_field("leverage")) {
                            futurePosition.leverage = stoi(position.at("leverage").as_string());
                        }
                        if (position.has_field("risk_limit")) {
                            futurePosition.riskLimit = stoi(position.at("risk_limit").as_string());
                        }
                        if (position.has_field("leverage_max")) {
                            futurePosition.leverageMax = stoi(position.at("leverage_max").as_string());
                        }
                        if (position.has_field("maintenance_rate")) {
                            futurePosition.maintenanceRate = stod(position.at("maintenance_rate").as_string());
                        }
                        if (position.has_field("value")) {
                            futurePosition.value = stod(position.at("value").as_string());
                        }
                        if (position.has_field("margin")) {
                            futurePosition.margin = stod(position.at("margin").as_string());
                        }
                        if (position.has_field("entry_price")) {
                            futurePosition.entryPrice = stod(position.at("entry_price").as_string());
                        }
                        if (position.has_field("liq_price")) {
                            futurePosition.liqPrice = stod(position.at("liq_price").as_string());
                        }
                        if (position.has_field("mark_price")) {
                            futurePosition.markPrice = stod(position.at("mark_price").as_string());
                        }
                        if (position.has_field("unrealised_pnl")) {
                            futurePosition.unrealisedPnl = stod(position.at("unrealised_pnl").as_string());
                        }
                        if (position.has_field("realised_pnl")) {
                            futurePosition.realisedPnl = stod(position.at("realised_pnl").as_string());
                        }
                        if (position.has_field("history_pnl")) {
                            futurePosition.historyPnl = stod(position.at("history_pnl").as_string());
                        }
                        if (position.has_field("last_close_pnl")) {
                            futurePosition.lastClosePnl = stod(position.at("last_close_pnl").as_string());
                        }
                        if (position.has_field("realised_point")) {
                            futurePosition.realisedPoint = stod(position.at("realised_point").as_string());
                        }
                        if (position.has_field("history_point")) {
                            futurePosition.historyPoint = stod(position.at("history_point").as_string());
                        }
                        if (fabs(futurePosition.size) > 0.0000000001) {
            		    LOG_INFO("QueryPosition AccountId: %d   GateioPerpetual position: %s", accountInfo.accountId, futurePosition.toString().c_str());
                            vFuturePosition.emplace_back(futurePosition);
                        }
                    }
                }            

	        if (content.has_field("label")) {
                    string label = content.at("label").as_string();
		    if (label != "USER_NOT_FOUND") {
                    	query = false;
                    	stringstream ss;
                    	string msg = "";
                    	if (label.size() > 0) {
                            if (content.has_field("message")) {
                                msg = content.at("message").as_string();
                            } else {
                                msg = content.at("label").as_string();
                            }
                        }
                        ss << "GateioPerpetual QueryPosition msg:" << msg;
                        string errMsg = ss.str();
                        LOG_INFO("QueryPosition AccountId: %d   GateioPerpetual Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                        vErrorMsg.emplace_back(errMsg);
		    }
                }

            })
            .wait();
        } catch(exception& e) {
            query = false;
            string errMsg = string("GateioPerpetual QueryPosition ") + string(e.what());
            LOG_INFO("QueryPosition AccountId: %d   GateioPerpetual Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

bool GateioPerpetual::QuerySettleOpenOrder(string settle, vector<gateio::FutureOrder>& vFutureOrder, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vFutureOrder.clear();
        vErrorMsg.clear();

        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            string orderSettleUrl = orderUrl + settle + "/orders";
            uri_builder builder(orderSettleUrl);
            builder.append_query("status", "open");

            string hashStr = sha512("");
            int64_t timestamp = gettickcount() / 1000;

            string queryStr = builder.to_string();
            string queryBody = "";
            size_t pos = queryStr.find("?");
            if (pos != string::npos) {
                queryBody = queryStr.substr(pos + 1, queryStr.size());
            }

            stringstream ss;
            ss << "GET" << "\n" << orderSettleUrl << "\n" << queryBody << "\n" << hashStr << "\n" << timestamp;

            string message = ss.str();
            string signature = getSignatureGate(message, accountInfo.secretKey);
            request.headers().add("Accept", "application/json");  
            request.headers().add("Content-Type", "application/json");  
            request.headers().add("SIGN", signature);  
            request.headers().add("Timestamp", timestamp);
            request.headers().add("KEY", accountInfo.apiKey);
            

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if (code == status_codes::OK || code == status_codes::BadRequest || code == status_codes::NotFound) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                    return response.extract_json();
                }
                
                LOG_INFO("GateioPerpetual QueryOpenOrder response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.is_array()) {
                    auto openOrderArray = content.as_array();
                    for (auto& openOrder: openOrderArray) {
                        gateio::FutureOrder futureOrder;
                        if (openOrder.has_field("contract")) {
                            futureOrder.contract = openOrder.at("contract").as_string();
                        }
                        if (openOrder.has_field("create_time")) {
                            futureOrder.createTime = openOrder.at("create_time").as_number().to_int64();
                        }
                        if (openOrder.has_field("finish_time")) {
                            futureOrder.finishTime = openOrder.at("finish_time").as_number().to_int64();
                        }
                        if (openOrder.has_field("size")) {
                            futureOrder.size = openOrder.at("size").as_integer();
                        }
                        if (openOrder.has_field("left")) {
                            futureOrder.left = openOrder.at("left").as_integer();
                        }
                        if (openOrder.has_field("price")) {
                            futureOrder.price = stod(openOrder.at("price").as_string());
                        }
                        if (openOrder.has_field("fill_price")) {
                            futureOrder.fillPrice = stod(openOrder.at("fill_price").as_string());
                        }
                        if (openOrder.has_field("mkfr")) {
                            futureOrder.mkfr = stod(openOrder.at("mkfr").as_string());
                        }
                        if (openOrder.has_field("tkfr")) {
                            futureOrder.tkfr = stod(openOrder.at("tkfr").as_string());
                        }
                        if (openOrder.has_field("status")) {
                            futureOrder.status = openOrder.at("status").as_string();
                        }
                        if (openOrder.has_field("finishAs")) {
                            futureOrder.finishAs = openOrder.at("finishAs").as_string();
                        }

            		LOG_INFO("QueryOpenOrder AccountId: %d   GateioPerpetual order: %s", accountInfo.accountId, futureOrder.toString().c_str());
                        vFutureOrder.emplace_back(futureOrder);
                    }
                }

	        if (content.has_field("label")) {
                    string label = content.at("label").as_string();
		    if (label != "USER_NOT_FOUND") {
                    	query = false;
                    	stringstream ss;
                    	string msg = "";
                    	if (label.size() > 0) {
                            if (content.has_field("message")) {
                                msg = content.at("message").as_string();
                            } else {
                                msg = content.at("label").as_string();
                            }
                        }
                        ss << "GateioPerpetual QueryOpenOrder msg:" << msg;
                        string errMsg = ss.str();
                        LOG_INFO("QueryOpenOrder AccountId: %d   GateioPerpetual Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                        vErrorMsg.emplace_back(errMsg);
		    }
                }

            })
            .wait();
        } catch(exception& e) {
            query = false;
            string errMsg = string("GateioPerpetual QueryOpenOrder ") + string(e.what());
            LOG_INFO("QueryOpenOrder AccountId: %d   GateioPerpetual Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

bool GateioPerpetual::QuerySettleOrder(string settle, vector<gateio::FutureOrder>& vFutureOrder, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vFutureOrder.clear();
        vErrorMsg.clear();

        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            string orderSettleUrl = orderUrl + settle + "/orders_timerange";
            uri_builder builder(orderSettleUrl);
   
            string hashStr = sha512("");
            int64_t timestamp = gettickcount() / 1000;

            string queryStr = builder.to_string();
            string queryBody = "";
            size_t pos = queryStr.find("?");
            if (pos != string::npos) {
                queryBody = queryStr.substr(pos + 1, queryStr.size());
            }

            stringstream ss;
            ss << "GET" << "\n" << orderSettleUrl << "\n" << queryBody << "\n" << hashStr << "\n" << timestamp;

            string message = ss.str();
            string signature = getSignatureGate(message, accountInfo.secretKey);
            request.headers().add("Accept", "application/json");  
            request.headers().add("Content-Type", "application/json");  
            request.headers().add("SIGN", signature);  
            request.headers().add("Timestamp", timestamp);
            request.headers().add("KEY", accountInfo.apiKey);
            

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if (code == status_codes::OK || code == status_codes::BadRequest || code == status_codes::NotFound) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                    return response.extract_json();
                }
                
                LOG_INFO("GateioPerpetual QueryOrder response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.is_array()) {
                    auto openOrderArray = content.as_array();
                    for (auto& openOrder: openOrderArray) {
                        gateio::FutureOrder futureOrder;
                        if (openOrder.has_field("contract")) {
                            futureOrder.contract = openOrder.at("contract").as_string();
                        }
                        if (openOrder.has_field("create_time")) {
                            futureOrder.createTime = openOrder.at("create_time").as_number().to_int64();
                        }
                        if (openOrder.has_field("finish_time")) {
                            futureOrder.finishTime = openOrder.at("finish_time").as_number().to_int64();
                        }
                        if (openOrder.has_field("size")) {
                            futureOrder.size = openOrder.at("size").as_integer();
                        }
                        if (openOrder.has_field("left")) {
                            futureOrder.left = openOrder.at("left").as_integer();
                        }
                        if (openOrder.has_field("price")) {
                            futureOrder.price = stod(openOrder.at("price").as_string());
                        }
                        if (openOrder.has_field("fill_price")) {
                            futureOrder.fillPrice = stod(openOrder.at("fill_price").as_string());
                        }
                        if (openOrder.has_field("mkfr")) {
                            futureOrder.mkfr = stod(openOrder.at("mkfr").as_string());
                        }
                        if (openOrder.has_field("tkfr")) {
                            futureOrder.tkfr = stod(openOrder.at("tkfr").as_string());
                        }
                        if (openOrder.has_field("status")) {
                            futureOrder.status = openOrder.at("status").as_string();
                        }
                        if (openOrder.has_field("finishAs")) {
                            futureOrder.finishAs = openOrder.at("finishAs").as_string();
                        }

            		    LOG_INFO("QueryOrder AccountId: %d   GateioPerpetual order: %s", accountInfo.accountId, futureOrder.toString().c_str());
                        if (futureOrder.finishAs == "liquidated" || futureOrder.finishAs == "auto_deleveraged") {
                            vFutureOrder.emplace_back(futureOrder);
                        }
                        
                    }
                }

	        if (content.has_field("label")) {
                    string label = content.at("label").as_string();
		    if (label != "USER_NOT_FOUND") {
                    	query = false;
                    	stringstream ss;
                    	string msg = "";
                    	if (label.size() > 0) {
                            if (content.has_field("message")) {
                                msg = content.at("message").as_string();
                            } else {
                                msg = content.at("label").as_string();
                            }
                        }
                        ss << "GateioPerpetual QueryOrder msg:" << msg;
                        string errMsg = ss.str();
                        LOG_INFO("QueryOrder AccountId: %d   GateioPerpetual Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                        vErrorMsg.emplace_back(errMsg);
		    }
                }

            })
            .wait();
        } catch(exception& e) {
            query = false;
            string errMsg = string("GateioPerpetual QueryOrder ") + string(e.what());
            LOG_INFO("QueryOrder AccountId: %d   GateioPerpetual Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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
