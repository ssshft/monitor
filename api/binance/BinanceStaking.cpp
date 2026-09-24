#include "binance/BinanceStaking.h"
#include "Net.h"


BinanceStaking::BinanceStaking(AccountInfo& info) {
    positionUrl = "/sapi/v1/staking/position";
    accountInfo = info;
}

BinanceStaking::~BinanceStaking() {
}

bool BinanceStaking::QueryPosition(std::vector<binance::StakingPosition> vStakingPosition, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string qs = fmt::format("recvWindow={}&timestamp={}", 5000, crypto::getCurrentTimeMilli());
    std::string sig = crypto::getBinanceSignatureRest(accountInfo.secretKey, qs);
    std::string fullPath = fmt::format("{}?{}&signature={}", positionUrl, qs, sig);

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), fullPath, {{"X-MBX-APIKEY", accountInfo.apiKey}}, {}, body, status)) {
            std::string errMsg = "BinanceStaking QueryPosition syncGet return false";
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("BinanceStaking QueryPosition status: {}", status);
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                binance::StakingPosition stakingPosition;
                if (res[i].HasMember("positionId")) {
                    stakingPosition.positionId = res[i]["positionId"].GetString();
                }
                if (res[i].HasMember("projectId")) {
                    stakingPosition.projectId = res[i]["projectId"].GetString();
                }
                if (res[i].HasMember("asset")) {
                    stakingPosition.asset = res[i]["asset"].GetString();
                }
                if (res[i].HasMember("amount")) {
                    stakingPosition.amount = std::stod(res[i]["amount"].GetString());
                }
                if (res[i].HasMember("purchaseTime")) {
                    stakingPosition.purchaseTime = std::stoll(res[i]["purchaseTime"].GetString());
                }
                if (res[i].HasMember("duration")) {
                    stakingPosition.duration = std::stoi(res[i]["duration"].GetString());
                }
                if (res[i].HasMember("accrualDays")) {
                    stakingPosition.accrualDays = std::stoi(res[i]["accrualDays"].GetString());
                }
                if (res[i].HasMember("rewardAsset")) {
                    stakingPosition.rewardAsset = res[i]["rewardAsset"].GetString();
                }
                if (res[i].HasMember("APY")) {
                    stakingPosition.apy = std::stod(res[i]["APY"].GetString());
                }
                if (res[i].HasMember("rewardAmt")) {
                    stakingPosition.rewardAmt = std::stod(res[i]["rewardAmt"].GetString());
                }
                if (res[i].HasMember("extraRewardAsset")) {
                    stakingPosition.extraRewardAsset = res[i]["extraRewardAsset"].GetString();
                }
                if (res[i].HasMember("extraRewardAPY")) {
                    stakingPosition.extraRewardApy = std::stod(res[i]["extraRewardAPY"].GetString());
                }
                if (res[i].HasMember("estExtraRewardAmt")) {
                    stakingPosition.estExtraRewardAmt = std::stod(res[i]["estExtraRewardAmt"].GetString());
                }
                if (res[i].HasMember("nextInterestPay")) {
                    stakingPosition.nextInterestPay = std::stod(res[i]["nextInterestPay"].GetString());
                }
                if (res[i].HasMember("nextInterestPayDate")) {
                    stakingPosition.nextInterestPayDate = std::stoll(res[i]["nextInterestPayDate"].GetString());
                }
                if (res[i].HasMember("payInterestPeriod")) {
                    stakingPosition.payInterestPeriod = std::stoi(res[i]["payInterestPeriod"].GetString());
                }
                if (res[i].HasMember("redeemAmountEarly")) {
                    stakingPosition.redeemAmountEarly = std::stod(res[i]["redeemAmountEarly"].GetString());
                }
                if (res[i].HasMember("interestEndDate")) {
                    stakingPosition.interestEndDate = std::stoll(res[i]["interestEndDate"].GetString());
                }
                if (res[i].HasMember("deliverDate")) {
                    stakingPosition.deliverDate = std::stoll(res[i]["deliverDate"].GetString());
                }
                if (res[i].HasMember("redeemPeriod")) {
                    stakingPosition.redeemPeriod = std::stoi(res[i]["redeemPeriod"].GetString());
                }
                if (res[i].HasMember("redeemingAmt")) {
                    stakingPosition.redeemingAmt = std::stod(res[i]["redeemingAmt"].GetString());
                }
                if (res[i].HasMember("partialAmtDeliverDate")) {
                    stakingPosition.partialAmtDeliverDate = std::stoll(res[i]["partialAmtDeliverDate"].GetString());
                }
                if (res[i].HasMember("canRedeemEarly")) {
                    stakingPosition.canRedeemEarly = res[i]["canRedeemEarly"].GetBool();
                }
                if (res[i].HasMember("renewable")) {
                    stakingPosition.renewable = res[i]["renewable"].GetBool();
                }
                if (res[i].HasMember("type")) {
                    stakingPosition.type = res[i]["type"].GetString();
                }
                if (res[i].HasMember("status")) {
                    stakingPosition.status = res[i]["status"].GetString();
                }
                    
                LOG_INFO("QueryPosition AccountId: {}  stakingPosition: {}", accountInfo.accountId, stakingPosition.toString());
                vStakingPosition.emplace_back(stakingPosition);
            }
        }
        else if (res.HasMember("code")) {
            query = false;
            int code = std::stoi(res["code"].GetString());
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("BinanceStaking QueryPosition code: {}, msg: {}", code, msg);
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("BinanceStaking QueryPosition exception: {}", e.what());
        LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}
