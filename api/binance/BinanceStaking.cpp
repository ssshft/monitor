#include "BinanceStaking.h"


BinanceStaking::BinanceStaking(AccountInfo& info) {
    positionUrl = "/sapi/v1/staking/position";
    accountInfo = info;
}

BinanceStaking::~BinanceStaking() {
}

vector<binance::StakingPosition> BinanceStaking::QueryPosition() {
    vector<binance::StakingPosition> v;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(accountInfo.restUrl, config);
        http_request request(methods::GET);
        request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
        uri_builder builder(positionUrl);

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
                
            LOG_INFO("BinanceStaking response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();
            if (content.is_array()) {
                auto positionArray = content.as_array();
                for(auto& position : positionArray) {
                    binance::StakingPosition stakingPosition;
                    if (position.has_field("positionId")) {
                        stakingPosition.positionId = position.at("positionId").as_string();
                    }
                    if (position.has_field("projectId")) {
                        stakingPosition.projectId = position.at("projectId").as_string();
                    }
                    if (position.has_field("asset")) {
                        stakingPosition.asset = position.at("asset").as_string();
                    }
                    if (position.has_field("amount")) {
                        stakingPosition.amount = stod(position.at("amount").as_string());
                    }
                    if (position.has_field("purchaseTime")) {
                        stakingPosition.purchaseTime = stoll(position.at("purchaseTime").as_string());
                    }
                    if (position.has_field("duration")) {
                        stakingPosition.duration = stoi(position.at("duration").as_string());
                    }
                    if (position.has_field("accrualDays")) {
                        stakingPosition.accrualDays = stoi(position.at("accrualDays").as_string());
                    }
                    if (position.has_field("rewardAsset")) {
                        stakingPosition.rewardAsset = position.at("rewardAsset").as_string();
                    }
                    if (position.has_field("APY")) {
                        stakingPosition.apy = stod(position.at("APY").as_string());
                    }
                    if (position.has_field("rewardAmt")) {
                        stakingPosition.rewardAmt = stod(position.at("rewardAmt").as_string());
                    }
                    if (position.has_field("extraRewardAsset")) {
                        stakingPosition.extraRewardAsset = position.at("extraRewardAsset").as_string();
                    }
                    if (position.has_field("extraRewardAPY")) {
                        stakingPosition.extraRewardApy = stod(position.at("extraRewardAPY").as_string());
                    }
                    if (position.has_field("estExtraRewardAmt")) {
                        stakingPosition.estExtraRewardAmt = stod(position.at("estExtraRewardAmt").as_string());
                    }
                    if (position.has_field("nextInterestPay")) {
                        stakingPosition.nextInterestPay = stod(position.at("nextInterestPay").as_string());
                    }
                    if (position.has_field("nextInterestPayDate")) {
                        stakingPosition.nextInterestPayDate = stoll(position.at("nextInterestPayDate").as_string());
                    }
                    if (position.has_field("payInterestPeriod")) {
                        stakingPosition.payInterestPeriod = stoi(position.at("payInterestPeriod").as_string());
                    }
                    if (position.has_field("redeemAmountEarly")) {
                        stakingPosition.redeemAmountEarly = stod(position.at("redeemAmountEarly").as_string());
                    }
                    if (position.has_field("interestEndDate")) {
                        stakingPosition.interestEndDate = stoll(position.at("interestEndDate").as_string());
                    }
                    if (position.has_field("deliverDate")) {
                        stakingPosition.deliverDate = stoll(position.at("deliverDate").as_string());
                    }
                    if (position.has_field("redeemPeriod")) {
                        stakingPosition.redeemPeriod = stoi(position.at("redeemPeriod").as_string());
                    }
                    if (position.has_field("redeemingAmt")) {
                        stakingPosition.redeemingAmt = stod(position.at("redeemingAmt").as_string());
                    }
                    if (position.has_field("partialAmtDeliverDate")) {
                        stakingPosition.partialAmtDeliverDate = stoll(position.at("partialAmtDeliverDate").as_string());
                    }
                    if (position.has_field("canRedeemEarly")) {
                        stakingPosition.canRedeemEarly = position.at("canRedeemEarly").as_bool();
                    }
                    if (position.has_field("renewable")) {
                        stakingPosition.renewable = position.at("renewable").as_bool();
                    }
                    if (position.has_field("type")) {
                        stakingPosition.type = position.at("type").as_string();
                    }
                    if (position.has_field("status")) {
                        stakingPosition.status = position.at("status").as_string();
                    }
                        
                    v.emplace_back(stakingPosition);
                }
            }
  
        })
        .wait();
    } catch(exception& e) {
        LOG_INFO("AccountId: %d   BinanceStaking Error: '%s' ", accountInfo.accountId, e.what());
    }

    return v;
}
