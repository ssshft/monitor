#include "BinanceMargin.h"


BinanceMargin::BinanceMargin(AccountInfo& info) {
    accountUrl = "/sapi/v1/margin/account";
    accountInfo = info;
}

BinanceMargin::~BinanceMargin() {
}

bool BinanceMargin::QueryAccount(vector<binance::MarginAsset>& vMarginAsset, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vMarginAsset.clear();
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

                LOG_INFO("QueryAccount AccountId: %d  BinanceMargin response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.has_field("marginLevel")) {
                    totalMarginAsset.marginLevel = stod(content.at("marginLevel").as_string());
                }

                if (content.has_field("totalAssetOfBtc")) {
                    totalMarginAsset.totalAssetOfBtc = stod(content.at("totalAssetOfBtc").as_string());
                }

                if (content.has_field("totalLiabilityOfBtc")) {
                    totalMarginAsset.totalLiabilityOfBtc = stod(content.at("totalLiabilityOfBtc").as_string());
                }

                if (content.has_field("totalNetAssetOfBtc")) {
                    totalMarginAsset.totalNetAssetOfBtc = stod(content.at("totalNetAssetOfBtc").as_string());
                }

                LOG_INFO("QueryAccount totalMarginAsset AccountId: %d    BinanceMargin: %s", accountInfo.accountId, totalMarginAsset.toString().c_str());

                if (content.has_field("userAssets")) {
                    json::value const& userAssets = content.at("userAssets");
                    if (userAssets.is_array()) {
                        auto assetArray = userAssets.as_array();
                        for (auto& asset : assetArray) {
                            binance::MarginAsset marginAsset;
                            if (asset.has_field("asset")) {
                                marginAsset.asset = asset.at("asset").as_string();
                            }
                            if (asset.has_field("borrowed")) {
                                marginAsset.borrowed = stod(asset.at("borrowed").as_string());
                            }
                            if (asset.has_field("free")) {
                                marginAsset.free = stod(asset.at("free").as_string());
                            }
                            if (asset.has_field("interest")) {
                                marginAsset.interest = stod(asset.at("interest").as_string());
                            }
                            if (asset.has_field("locked")) {
                                marginAsset.locked = stod(asset.at("locked").as_string());
                            }
                            if (asset.has_field("netAsset")) {
                                marginAsset.netAsset = stod(asset.at("netAsset").as_string());
                            }

                            if (fabs(marginAsset.borrowed) <= 0.0000000001 && fabs(marginAsset.free) <= 0.0000000001 && fabs(marginAsset.interest) <= 0.0000000001 && fabs(marginAsset.locked) <= 0.0000000001 && fabs(marginAsset.netAsset) <= 0.0000000001) {
                                continue;
                            }

                            LOG_INFO("QueryAccount marginAsset AccountId: %d    BinanceMargin: %s", accountInfo.accountId, marginAsset.toString().c_str());

                            vMarginAsset.emplace_back(marginAsset);
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
                    ss << "BinanceMargin QueryAccount code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   BinanceMargin Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }

            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "BinanceMargin QueryAccount code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryAccount AccountId: %d   BinanceMargin Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
        */
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceMargin QueryAccount ") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   BinanceMargin Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

binance::TotalMarginAsset BinanceMargin::GetTotalAsset() {
    return totalMarginAsset;
}
