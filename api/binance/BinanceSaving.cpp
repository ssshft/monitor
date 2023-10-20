#include "BinanceSaving.h"


BinanceSaving::BinanceSaving(AccountInfo& info) {
    accountUrl = "/sapi/v1/lending/union/account";
    interestUrl = "/sapi/v1/lending/union/interestHistory";
    purchaseUrl = "/sapi/v1/lending/union/purchaseRecord";
    redemptionUrl = "/sapi/v1/lending/union/redemptionRecord";
    accountInfo = info;
}

BinanceSaving::~BinanceSaving() {
}

bool BinanceSaving::QueryAccount(vector<binance::SavingAsset>& vSavingAsset, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vSavingAsset.clear();
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

                LOG_INFO("QueryAccount AccountId: %d  BinanceSaving response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.has_field("positionAmountVos")) {
                    json::value const& positionAmountVos = content.at("positionAmountVos");
                    if (positionAmountVos.is_array()) {
                        auto assetArray = positionAmountVos.as_array();
                        for(auto& asset : assetArray) {
                            binance::SavingAsset savingAsset;
                            if (asset.has_field("asset")) {
                                savingAsset.asset = asset.at("asset").as_string();
                            }
                            if (asset.has_field("amount")) {
                                savingAsset.amount = stod(asset.at("amount").as_string());
                            }
                            if (asset.has_field("amountInBTC")) {
                                savingAsset.amountInBTC = stod(asset.at("amountInBTC").as_string());
                            }
                            if (asset.has_field("amountInUSDT")) {
                                savingAsset.amountInUSDT = stod(asset.at("amountInUSDT").as_string());
                            }
                        
                            if (fabs(savingAsset.amount) <= 0.0000000001) {
                                continue;
                            }
                            vSavingAsset.emplace_back(savingAsset);
                    	    LOG_INFO("BinanceSaving AccountId: %d QueryAccount:%s", accountInfo.accountId, savingAsset.toString().c_str());
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
                    ss << "BinanceSaving QueryAccount code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   BinanceSaving Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }

            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "BinanceSaving QueryAccount code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryAccount AccountId: %d   BinanceSaving Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
        */
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceSaving QueryAccount ") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   BinanceSaving Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

vector<binance::SavingInterest> BinanceSaving::QueryInterest() {
    vector<binance::SavingInterest> v;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(accountInfo.restUrl, config);
        http_request request(methods::GET);
        request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);

        vector<string> vLendingType = {"DAILY", "ACTIVITY", "CUSTOMIZED_FIXED"};
        for (size_t i = 0; i < vLendingType.size(); ++i) {
            uri_builder builder(interestUrl);
            int64_t currentTime = gettickcount();
            builder.append_query("lendingType", vLendingType[i]);
            builder.append_query("recvWindow", 5000);
            builder.append_query("timestamp", currentTime);
            auto signature = getSignature(builder.query(), accountInfo.secretKey);
            builder.append_query("signature", signature);    

            //LOG_INFO("BinanceSaving QueryInterest: '%s' ", builder.to_string().c_str());

            request.set_request_uri(builder.to_string());
            client.request(request)
            .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if(code == status_codes::OK || code == status_codes::BadRequest) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                    return response.extract_json();
                }
                
                LOG_INFO("BinanceSaving response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.is_array()) {
                    auto interestArray = content.as_array();
                    for(auto& interest : interestArray) {
                        binance::SavingInterest savingInterest;
                        if (interest.has_field("symbol")) {
                            savingInterest.asset = interest.at("symbol").as_string();
                        }
                        if (interest.has_field("interest")) {
                            savingInterest.interest = stod(interest.at("interest").as_string());
                        }
                        if (interest.has_field("lendingType")) {
                            savingInterest.lendingType = interest.at("lendingType").as_string();
                        }
                        if (interest.has_field("productName")) {
                            savingInterest.productName = interest.at("productName").as_string();
                        }
                        if (interest.has_field("time")) {
                            savingInterest.time = interest.at("time").as_number().to_int64();
                        }
                        
                        v.emplace_back(savingInterest);
                    }
                }
            })
            .wait();
            sleep(1);
        }
    } catch(exception& e) {
        LOG_INFO("BinanceSaving Error: '%s' ", e.what());
    }

    return v;
}

vector<binance::SavingPurchase> BinanceSaving::QueryPurchase() {
    vector<binance::SavingPurchase> v;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(accountInfo.restUrl, config);
        http_request request(methods::GET);
        request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);

        vector<string> vLendingType = {"DAILY", "ACTIVITY", "CUSTOMIZED_FIXED"};
        for (size_t i = 0; i < vLendingType.size(); ++i) {
            uri_builder builder(purchaseUrl);
            int64_t currentTime = gettickcount();
            builder.append_query("lendingType", vLendingType[i]);
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
                
                LOG_INFO("BinanceSaving response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.is_array()) {
                    auto purchaseArray = content.as_array();
                    for(auto& purchase : purchaseArray) {
                        binance::SavingPurchase savingPurchase;
                        if (purchase.has_field("symbol")) {
                            savingPurchase.asset = purchase.at("symbol").as_string();
                        }
                        if (purchase.has_field("amount")) {
                            savingPurchase.amount = stod(purchase.at("amount").as_string());
                        }
                        if (purchase.has_field("lendingType")) {
                            savingPurchase.lendingType = purchase.at("lendingType").as_string();
                        }
                        if (purchase.has_field("productName")) {
                            savingPurchase.productName = purchase.at("productName").as_string();
                        }
                        if (purchase.has_field("lot")) {
                            savingPurchase.lot = purchase.at("lot").as_integer();
                        }
                        if (purchase.has_field("purchaseId")) {
                            savingPurchase.purchaseId = purchase.at("purchaseId").as_integer();
                        }
                        if (purchase.has_field("status")) {
                            savingPurchase.status = purchase.at("status").as_string();
                        }
                        if (purchase.has_field("createTime")) {
                            savingPurchase.createTime = purchase.at("createTime").as_number().to_int64();
                        }
                    
                        v.emplace_back(savingPurchase);
                    }
                }
            })
            .wait();
            sleep(1);
        }
    } catch(exception& e) {
        LOG_INFO("BinanceSaving Error: '%s' ", e.what());
    }

    return v;
}

vector<binance::SavingRedemption> BinanceSaving::QueryRedemption() {
    vector<binance::SavingRedemption> v;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(accountInfo.restUrl, config);
        http_request request(methods::GET);
        request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);

        vector<string> vLendingType = {"DAILY", "ACTIVITY", "CUSTOMIZED_FIXED"};
        for (size_t i = 0; i < vLendingType.size(); ++i) {
            uri_builder builder(redemptionUrl);
            int64_t currentTime = gettickcount();
            builder.append_query("lendingType", vLendingType[i]);
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
                
                LOG_INFO("BinanceSaving response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.is_array()) {
                    auto redemptionArray = content.as_array();
                    for(auto& redemption : redemptionArray) {
                        binance::SavingRedemption savingRedemption;
                        if (redemption.has_field("symbol")) {
                            savingRedemption.asset = redemption.at("symbol").as_string();
                        }
                        if (redemption.has_field("amount")) {
                            savingRedemption.amount = stod(redemption.at("amount").as_string());
                        }
                        if (redemption.has_field("principal")) {
                            savingRedemption.principal = stod(redemption.at("principal").as_string());
                        }
                        if (redemption.has_field("projectId")) {
                            savingRedemption.projectId = redemption.at("projectId").as_integer();
                        }
                        if (redemption.has_field("projectName")) {
                            savingRedemption.projectName = redemption.at("projectName").as_string();
                        }
                        if (redemption.has_field("status")) {
                            savingRedemption.status = redemption.at("status").as_string();
                        }
                        if (redemption.has_field("createTime")) {
                            savingRedemption.createTime = redemption.at("createTime").as_number().to_int64();
                        }
                        
                        v.emplace_back(savingRedemption);
                    }
                }
            })
            .wait();
            sleep(1);
        }
    } catch(exception& e) {
        LOG_INFO("BinanceSaving Error: '%s' ", e.what());
    }

    return v;
}
