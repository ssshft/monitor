#include "GateioCrossMargin.h"


GateioCrossMargin::GateioCrossMargin(AccountInfo& info) {
    accountUrl = "/api/v4/margin/cross/accounts";
    accountInfo = info;
}

GateioCrossMargin::~GateioCrossMargin() {
}

bool GateioCrossMargin::QueryAccount(vector<gateio::CrossMarginAsset>& vCrossMarginAsset, gateio::CrossMarginAccountTotal& crossMarginAccountTotal, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vCrossMarginAsset.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            uri_builder builder(accountUrl);
            
            string hashStr = sha512("");
            int64_t timestamp = gettickcount() / 1000;
            stringstream ss;
            ss << "GET" << "\n" << accountUrl << "\n" << "" << "\n" << hashStr << "\n" << timestamp;
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
                if(code == status_codes::OK || code == status_codes::BadRequest || code == status_codes::NotFound) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                    return response.extract_json();
                }
                
                LOG_INFO("GateioCrossMargin QueryAccount response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                //LOG_INFO("GateioCrossMargin QueryAccount AccountId: %d  content: %s", accountInfo.accountId, content.to_string().c_str());
                if (content.has_field("balances")) {
                    auto& balances = content.at("balances").as_object();
                    for (auto iter = balances.begin(); iter != balances.end(); ++iter) {
                        gateio::CrossMarginAsset crossMarginAsset;
                        crossMarginAsset.currency = iter->first;
                        auto& asset = iter->second;
                        if (asset.has_field("available")) {
                            crossMarginAsset.available = fabs(stod(asset.at("available").as_string()));
                        }
                        if (asset.has_field("freeze")) {
                            crossMarginAsset.freeze = fabs(stod(asset.at("freeze").as_string()));
                        }
                        if (asset.has_field("borrowed")) {
                            crossMarginAsset.borrowed = fabs(stod(asset.at("borrowed").as_string()));
                        }
                        if (asset.has_field("interest")) {
                            crossMarginAsset.interest = fabs(stod(asset.at("interest").as_string()));
                        }
                        if (asset.has_field("negative_liab")) {
                            crossMarginAsset.negativeLiab = fabs(stod(asset.at("negative_liab").as_string()));
                        }
                        if (asset.has_field("futures_pos_liab")) {
                            crossMarginAsset.futuresPosLiab = fabs(stod(asset.at("futures_pos_liab").as_string()));
                        }
                        if (asset.has_field("equity")) {
                            crossMarginAsset.equity = fabs(stod(asset.at("equity").as_string()));
                        }
                        if (asset.has_field("total_freeze")) {
                            crossMarginAsset.totalFreeze = fabs(stod(asset.at("total_freeze").as_string()));
                        }
                        if (asset.has_field("total_liab")) {
                            crossMarginAsset.totalLiab = fabs(stod(asset.at("total_liab").as_string()));
                        }

                        if ((crossMarginAsset.available + crossMarginAsset.freeze) >= 0.0000000001) {
                    	    LOG_INFO("QueryAccount AccountId: %d   GateioCrossMargin asset: %s", accountInfo.accountId, crossMarginAsset.toString().c_str());
                            vCrossMarginAsset.emplace_back(crossMarginAsset);
                        }
                    }
                }

                if (content.has_field("total")) {
                    crossMarginAccountTotal.total = stod(content.at("total").as_string());
                }

                if (content.has_field("borrowed")) {
                    crossMarginAccountTotal.borrowed = stod(content.at("borrowed").as_string());
                }

                if (content.has_field("interest")) {
                    crossMarginAccountTotal.interest = stod(content.at("interest").as_string());
                }

                if (content.has_field("risk")) {
                    crossMarginAccountTotal.risk = stod(content.at("risk").as_string());
                }

                if (content.has_field("total_initial_margin")) {
                    crossMarginAccountTotal.totalInitialMargin = stod(content.at("total_initial_margin").as_string());
                }

                if (content.has_field("total_margin_balance")) {
                    crossMarginAccountTotal.totalMarginBalance = stod(content.at("total_margin_balance").as_string());
                }

                if (content.has_field("total_maintenance_margin")) {
                    crossMarginAccountTotal.totalMaintenanceMargin = stod(content.at("total_maintenance_margin").as_string());
                }

                if (content.has_field("total_initial_margin_rate")) {
                    crossMarginAccountTotal.totalInitialMarginRate = stod(content.at("total_initial_margin_rate").as_string());
                }

                if (content.has_field("total_maintenance_margin_rate")) {
                    crossMarginAccountTotal.totalMaintenanceMarginRate = stod(content.at("total_maintenance_margin_rate").as_string());
                }

                if (content.has_field("total_available_margin")) {
                    crossMarginAccountTotal.totalAvailableMargin = stod(content.at("total_available_margin").as_string());
                }

                if (content.has_field("portfolio_margin_total")) {
                    crossMarginAccountTotal.portfolioMarginTotal = stod(content.at("portfolio_margin_total").as_string());
                }

                if (content.has_field("portfolio_margin_total_liab")) {
                    crossMarginAccountTotal.portfolioMarginTotalLiab = stod(content.at("portfolio_margin_total_liab").as_string());
                }

                if (content.has_field("portfolio_margin_total_equity")) {
                    crossMarginAccountTotal.portfolioMarginTotalEquity = stod(content.at("portfolio_margin_total_equity").as_string());
                }


	        if (content.has_field("label")) {
                    query = false;
                    stringstream ss;
                    string label = content.at("label").as_string();
                    string msg = "";
                    if (label.size() > 0) {
                        if (content.has_field("message")) {
                            msg = content.at("message").as_string();
                        } else {
                            msg = content.at("label").as_string();
                        }
                    }
                    ss << "GateioCrossMargin QueryAccount msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   GateioCrossMargin Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }

            })
            .wait();
        } catch(exception& e) {
            query = false;
            string errMsg = string("GateioCrossMargin QueryAccount") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   GateioCrossMargin Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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
