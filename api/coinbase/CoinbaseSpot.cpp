#include "CoinbaseSpot.h"


CoinbaseSpot::CoinbaseSpot(AccountInfo& info) {
    portfolioUrl = "/v1/portfolios";
    accountUrl = "";
    accountInfo = info;
    QueryPortfolio();
}

CoinbaseSpot::~CoinbaseSpot() {
}

void CoinbaseSpot::QueryPortfolio() {
    try {
        http_client restclient(accountInfo.restUrl);
        uri_builder builder(portfolioUrl);
        http_request request(methods::GET);

        int64_t timstamp = gettickcount() / 1000;  // 秒
        stringstream ss;
        ss << timstamp << "GET" << portfolioUrl;
        string message = ss.str();
        string signature = getSignatureNew(message, accountInfo.secretKey);
        request.headers().add("Accept", "application/json");  
        request.headers().add("X-CB-ACCESS-SIGNATURE", signature);  
        request.headers().add("X-CB-ACCESS-TIMESTAMP", timstamp);
        request.headers().add("X-CB-ACCESS-KEY", accountInfo.apiKey);
        request.headers().add("X-CB-ACCESS-PASSPHRASE", accountInfo.passphrase);

        request.set_request_uri(builder.to_string());
        restclient.request(request)
        .then([](http_response response) -> pplx::task<json::value> {
            if(response.status_code() == status_codes::OK) {
                return response.extract_json();
            }
            LOG_DEBUG("CoinbaseSpot QueryPortfolio response: '%s' ", response.to_string().c_str());
            return pplx::task_from_result(json::value());
        }) // continue when the JSON value is available
        .then([&](pplx::task<json::value> previousTask) { // get the JSON value from the task and display content from it
            try {
                json::value const& v = previousTask.get();
                if (v.has_field("portfolios")) {
                    json::value const & result = v.at("portfolios");
                    if (result.is_array()) {
                        auto portfolioArray = result.as_array();
                        for(auto& portfolio : portfolioArray) {
                            if (portfolio.has_field("id")) {
                                portfolioId = portfolio.at("id").as_string();
                            }

                            if (portfolio.has_field("entity_id")) {
                                entityId = portfolio.at("entity_id").as_string();
                            }

                            if (portfolio.has_field("organization_id")) {
                                organizationId = portfolio.at("organization_id").as_string();
                            }
                        }
                    }
                }
            } catch (http_exception const & e) {
                printf("Error exception:%s\n", e.what());
            }
        })
        .wait();
    } catch(exception& e) {
        LOG_DEBUG("Retrieve portfolios Error: '%s' ", e.what());
    }
}

bool CoinbaseSpot::QueryAccount(vector<coinbase::SpotAsset>& vSpotAsset, vector<string>& vErrorMsg) {
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

            accountUrl = "/v1/portfolios/" + portfolioId + "/balances";
            uri_builder builder(accountUrl);

            int64_t timstamp = gettickcount() / 1000;  // 秒
            stringstream ss;
            ss << timstamp << "GET" << accountUrl;
            string message = ss.str();
            string signature = getSignatureNew(message, accountInfo.secretKey);
            request.headers().add("Accept", "application/json");  
            request.headers().add("X-CB-ACCESS-SIGNATURE", signature);  
            request.headers().add("X-CB-ACCESS-TIMESTAMP", timstamp);
            request.headers().add("X-CB-ACCESS-KEY", accountInfo.apiKey);
            request.headers().add("X-CB-ACCESS-PASSPHRASE", accountInfo.passphrase);

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

                LOG_INFO("QueryAccount AccountId: %d  CoinbaseSpot response: '%s' ", accountInfo.accountId, response.to_string().c_str());
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
                            coinbase::SpotAsset spotAsset;
                            if (asset.has_field("symbol")) {
                                string symbol = asset.at("symbol").as_string();
                                spotAsset.symbol = boost::to_upper_copy(symbol);
                            }
                            if (asset.has_field("amount")) {
                                spotAsset.amount = stod(asset.at("amount").as_string());
                            }
                            if (asset.has_field("holds")) {
                                spotAsset.holds = stod(asset.at("holds").as_string());
                            }
                            if (asset.has_field("bonded_amount")) {
                                spotAsset.bondedAmount = stod(asset.at("bonded_amount").as_string());
                            }
                            if (asset.has_field("reserved_amount")) {
                                spotAsset.reservedAmount = stod(asset.at("reserved_amount").as_string());
                            }
                            if (asset.has_field("unbonding_amount")) {
                                spotAsset.unbondingAmount = stod(asset.at("unbonding_amount").as_string());
                            }
                            if (asset.has_field("unvested_amount")) {
                                spotAsset.unvestedAmount = stod(asset.at("unvested_amount").as_string());
                            }
                            if (asset.has_field("pending_rewards_amount")) {
                                spotAsset.pendingRewardsAmount = stod(asset.at("pending_rewards_amount").as_string());
                            }
                            if (asset.has_field("past_rewards_amount")) {
                                spotAsset.pastRewardsAmount = stod(asset.at("past_rewards_amount").as_string());
                            }
                            if (asset.has_field("bondable_amount")) {
                                spotAsset.bondableAmount = stod(asset.at("bondable_amount").as_string());
                            }
                            if (asset.has_field("withdrawable_amount")) {
                                spotAsset.withdrawableAmount = stod(asset.at("withdrawable_amount").as_string());
                            }

                            if (spotAsset.amount <= 0.0000000001) {
                                continue;
                            }

                    	    LOG_INFO("QueryAccount AccountId: %d   CoinbaseSpot Asset: %s", accountInfo.accountId, spotAsset.toString().c_str());
                            vSpotAsset.emplace_back(spotAsset);
                        }
                    }
                }

                if (content.has_field("error")) {
                    query = false;
                    int code = 0;
                    string msg = "";
                    json::value const & error = content.at("error");
                    if (error.has_field("code")) {
                        code = error.at("code").as_integer();
                    }   

                    if (error.has_field("message")) {
                        msg = error.at("message").as_string();
                    } 
                    stringstream ss;
                    ss << "CoinbaseSpot QueryAccount code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   CoinbaseSpot Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "CoinbaseSpot QueryAccount code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryAccount AccountId: %d   CoinbaseSpot Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
        */
        catch(exception& e) {
            query = false;
            string errMsg = string("CoinbaseSpot QueryAccount ") + string(e.what());
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
