#include "BinanceBSwap.h"


BinanceBSwap::BinanceBSwap(AccountInfo& info) {
	liquidityUrl = "/sapi/v1/bswap/liquidity";
    unclaimedRewardsUrl = "/sapi/v1/bswap/unclaimedRewards";
    accountInfo = info;
}

BinanceBSwap::~BinanceBSwap() {
}

vector<binance::LiquidityInfo> BinanceBSwap::QueryLiquidity() {
    vector<binance::LiquidityInfo> v;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(accountInfo.restUrl, config);
        http_request request(methods::GET);
        request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
        uri_builder builder(liquidityUrl);

        builder.append_query("recvWindow", 5000);
        builder.append_query("timestamp", gettickcount());
        auto signature = getSignature(builder.query(), accountInfo.secretKey);
        builder.append_query("signature", signature);    

        request.set_request_uri(builder.to_string());
        client.request(request)
        .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
            auto code = response.status_code();
            if(code == status_codes::OK || code == status_codes::BadRequest) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                return response.extract_json();
            }
                
            LOG_INFO("BinanceBSwap response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();
            if (content.is_array()) {
                auto poolArray = content.as_array();
                for (auto& pool : poolArray) {
                    binance::LiquidityInfo liquidityInfo;
                    if (pool.has_field("poolId")) {
                        liquidityInfo.poolId = pool.at("poolId").as_integer();
                    }
                    if (pool.has_field("poolNmae")) {
                        liquidityInfo.poolName = pool.at("poolNmae").as_string();
                    }
                    if (pool.has_field("updateTime")) {
                        liquidityInfo.updateTime = pool.at("updateTime").as_number().to_int64();
                    }
                    if (pool.has_field("liquidity")) {
                        auto liquidity = pool.at("liquidity").as_object();
                        for (auto iter = liquidity.cbegin(); iter != liquidity.cend(); ++iter) {
                            string name = iter->first;
                            double value = iter->second.as_double();
                            liquidityInfo.mLiquidityAsset[name] = value;
                        }
                    }
                    if (pool.has_field("share")) {
                        auto share = pool.at("share");
                        if (share.has_field("shareAmount")) {
                            liquidityInfo.shareAmount = share.at("shareAmount").as_integer();
                        }
                        if (share.has_field("sharePercentage")) {
                            liquidityInfo.sharePercentage = share.at("sharePercentage").as_double();
                        }
                        if (share.has_field("asset")) {
                            auto asset = share.at("asset").as_object();
                            for (auto iter = asset.cbegin(); iter != asset.cend(); ++iter) {
                                string name = iter->first;
                                double value = iter->second.as_double();
                                liquidityInfo.mShareAsset[name] = value;
                            }
                        }
                    }
                    v.emplace_back(liquidityInfo);
                }
            }
        })
        .wait();
    } catch(exception& e) {
        LOG_INFO("BinanceBSwap Error: '%s' ", e.what());
    }

    return v;
}

binance::UnclaimedRewards BinanceBSwap::QueryUnclaimedRewards() {
    binance::UnclaimedRewards unclaimedRewards;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(accountInfo.restUrl, config);
        http_request request(methods::GET);
        request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);

        uri_builder builder(unclaimedRewardsUrl);
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
                
            LOG_INFO("BinanceBSwap response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();
            if (content.has_field("totalUnclaimedRewards")) {
                auto totalUnclaimedRewards = content.at("totalUnclaimedRewards").as_object();
                for (auto iter = totalUnclaimedRewards.cbegin(); iter != totalUnclaimedRewards.cend(); ++iter) {
                    string name = iter->first;
                    double value = iter->second.as_double();
                    unclaimedRewards.mTotalUnclaimedRewards[name] = value;
                }
            }
        })
        .wait();
    } catch(exception& e) {
        LOG_INFO("BinanceBSwap Error: '%s' ", e.what());
    }

    return unclaimedRewards;
}