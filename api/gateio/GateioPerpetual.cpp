#include "gateio/GateioPerpetual.h"
#include "Net.h"


GateioPerpetual::GateioPerpetual(AccountInfo& info) {
    accountUrl = "/api/v4/futures/usdt/accounts";
    positionUrl = "/api/v4/futures/usdt/positions";
    orderUrl = "/api/v4/futures/usdt/orders";
    accountInfo = info;
}

GateioPerpetual::~GateioPerpetual() {
}

bool GateioPerpetual::QueryAccount(std::vector<gateio::FutureAsset>& vFutureAsset, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string time_str = std::to_string(crypto::getCurrentTimeSeconds());    
    std::string sign = crypto::getGateioSignatureRest("GET", accountUrl, time_str, "", "", accountInfo.secretKey);
    std::vector<std::pair<std::string, std::string>> headers = {{"KEY", accountInfo.apiKey}, {"Timestamp", time_str}, {"SIGN", sign}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), accountUrl, {}, headers, body, status)) {
            std::string errMsg = "GateioPerpetual QueryAccount syncGet return false";
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("GateioPerpetual QueryAccount status: {}", status);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        gateio::FutureAsset futureAsset;
        if (res.HasMember("currency")) {
            futureAsset.currency = crypto::to_upper(res["currency"].GetString());
        }
        if (res.HasMember("total")) {
            futureAsset.total = std::stod(res["total"].GetString());
        }
        if (res.HasMember("unrealised_pnl")) {
            futureAsset.unrealisedPnl = std::stod(res["unrealised_pnl"].GetString());
        }  
        if (res.HasMember("position_margin")) {
            futureAsset.positionMargin = std::stod(res["position_margin"].GetString());
        }  
        if (res.HasMember("order_margin")) {
            futureAsset.orderMargin = std::stod(res["order_margin"].GetString());
        }  

        if (res.HasMember("available")) {
            futureAsset.available = std::stod(res["available"].GetString());
        }  
        if (res.HasMember("position_initial_margin")) {
            futureAsset.positionInitialMargin = std::stod(res["position_initial_margin"].GetString());
        }  
        if (res.HasMember("point")) {
            futureAsset.point = std::stod(res["point"].GetString());
        }  
        if (res.HasMember("bonus")) {
            futureAsset.bonus = std::stod(res["bonus"].GetString());
        }  
        if (res.HasMember("in_dual_mode")) {
            futureAsset.inDualMode = res["in_dual_mode"].GetBool();
        }
  
        if (futureAsset.total >= 0.0000000001) {
            LOG_INFO("QueryAccount AccountId: {}   GateioPerpetual asset: {}", accountInfo.accountId, futureAsset.toString());
            vFutureAsset.emplace_back(futureAsset);
        }

        if (res.HasMember("label")) {
            std::string label = res["label"].GetString();
            std::string msg = "";
            if (label != "USER_NOT_FOUND") {
                query = false;
                if (res.HasMember("message")) {
                    msg = res["message"].GetString();
                } else {
                    msg = label;
                }
            }

            std::string errMsg = fmt::format("GateioPerpetual QueryAccount msg: {}", msg);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        } 
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("GateioPerpetual QueryAccount exception: {}", e.what());
        LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool GateioPerpetual::QueryPosition(vector<gateio::FuturePosition>& vFuturePosition, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string time_str = std::to_string(crypto::getCurrentTimeSeconds());    
    std::string sign = crypto::getGateioSignatureRest("GET", positionUrl, time_str, "", "", accountInfo.secretKey);
    std::vector<std::pair<std::string, std::string>> headers = {{"KEY", accountInfo.apiKey}, {"Timestamp", time_str}, {"SIGN", sign}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), positionUrl, {}, headers, body, status)) {
            std::string errMsg = "GateioPerpetual QueryPosition syncGet return false";
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("GateioPerpetual QueryPosition status: {}", status);
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                gateio::FuturePosition futurePosition;

                if (res[i].HasMember("contract")) {
                    futurePosition.contract = crypto::to_upper(res[i]["contract"].GetString());
                }

                futurePosition.instrumentType = "usdt";

                if (res[i].HasMember("size")) {
                    futurePosition.size = std::stoll(res[i]["size"].GetString());
                }

                if (res[i].HasMember("leverage")) {
                    futurePosition.leverage = std::stoi(res[i]["leverage"].GetString());
                }

                if (res[i].HasMember("risk_limit")) {
                    futurePosition.riskLimit = std::stoi(res[i]["risk_limit"].GetString());
                }

                if (res[i].HasMember("leverage_max")) {
                    futurePosition.leverageMax = std::stoi(res[i]["leverage_max"].GetString());
                }
   
                if (res[i].HasMember("maintenance_rate")) {
                    futurePosition.maintenanceRate = std::stod(res[i]["maintenance_rate"].GetString());
                }
   
                if (res[i].HasMember("value")) {
                    futurePosition.value = std::stod(res[i]["value"].GetString());
                }

                if (res[i].HasMember("margin")) {
                    futurePosition.margin = std::stod(res[i]["margin"].GetString());
                }

                if (res[i].HasMember("entry_price")) {
                    futurePosition.entryPrice = std::stod(res[i]["entry_price"].GetString());
                }

                if (res[i].HasMember("liq_price")) {
                    futurePosition.liqPrice = std::stod(res[i]["liq_price"].GetString());
                }

                if (res[i].HasMember("mark_price")) {
                    futurePosition.markPrice = std::stod(res[i]["mark_price"].GetString());
                }

                if (res[i].HasMember("unrealised_pnl")) {
                    futurePosition.unrealisedPnl = std::stod(res[i]["unrealised_pnl"].GetString());
                }

        
                if (res[i].HasMember("realised_pnl")) {
                    futurePosition.realisedPnl = std::stod(res[i]["realised_pnl"].GetString());
                }


                if (res[i].HasMember("history_pnl")) {
                    futurePosition.historyPnl = std::stod(res[i]["history_pnl"].GetString());
                }


                if (res[i].HasMember("last_close_pnl")) {
                    futurePosition.lastClosePnl = std::stod(res[i]["last_close_pnl"].GetString());
                }

                if (res[i].HasMember("realised_point")) {
                    futurePosition.realisedPoint = std::stod(res[i]["realised_point"].GetString());
                }  
        
                if (res[i].HasMember("history_point")) {
                    futurePosition.historyPoint = std::stod(res[i]["history_point"].GetString());
                }  
       
                if (fabs(futurePosition.size) > 0.0000000001) {
                    LOG_INFO("QueryPosition AccountId: {}   GateioPerpetual position: {}", accountInfo.accountId, futurePosition.toString());
                    vFuturePosition.emplace_back(futurePosition);
                }
            }
        }

        if (res.HasMember("label")) {
            std::string label = res["label"].GetString();
            std::string msg = "";
            if (label != "USER_NOT_FOUND") {
                query = false;
                if (res.HasMember("message")) {
                    msg = res["message"].GetString();
                } else {
                    msg = label;
                }
            }

            std::string errMsg = fmt::format("GateioPerpetual QueryPosition msg: {}", msg);
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        } 
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("GateioPerpetual QueryPosition exception: {}", e.what());
        LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool GateioPerpetual::QueryOpenOrder(vector<gateio::FutureOrder>& vFutureOrder, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string time_str = std::to_string(crypto::getCurrentTimeSeconds());    
    std::string sign = crypto::getGateioSignatureRest("GET", orderUrl, time_str, "", "", accountInfo.secretKey);
    std::vector<std::pair<std::string, std::string>> headers = {{"KEY", accountInfo.apiKey}, {"Timestamp", time_str}, {"SIGN", sign}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), orderUrl, {}, headers, body, status)) {
            std::string errMsg = "GateioPerpetual QueryOpenOrder syncGet return false";
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("GateioPerpetual QueryOpenOrder status: {}", status);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                gateio::FutureOrder futureOrder;

                if (res[i].HasMember("contract")) {
                    futureOrder.contract = crypto::to_upper(res[i]["contract"].GetString());
                }

                if (res[i].HasMember("create_time")) {
                    futureOrder.createTime = std::stoll(res[i]["create_time"].GetString());
                }

                if (res[i].HasMember("finish_time")) {
                    futureOrder.finishTime = std::stoll(res[i]["finish_time"].GetString());
                }

                if (res[i].HasMember("size")) {
                    futureOrder.size = std::stoi(res[i]["size"].GetString());
                }

                if (res[i].HasMember("left")) {
                    futureOrder.left = std::stoi(res[i]["left"].GetString());
                }

                if (res[i].HasMember("price")) {
                    if (!crypto::str_cmp(res[i]["price"].GetString(), "")) {
                        futureOrder.price = std::stod(res[i]["price"].GetString());
                    }
                }

                if (res[i].HasMember("fill_price")) {
                    if (!crypto::str_cmp(res[i]["fill_price"].GetString(), "")) {
                        futureOrder.fillPrice = std::stod(res[i]["fill_price"].GetString());
                    }
                }

                if (res[i].HasMember("mkfr")) {
                    if (!crypto::str_cmp(res[i]["mkfr"].GetString(), "")) {
                        futureOrder.mkfr = std::stod(res[i]["mkfr"].GetString());
                    }
                }
   
                if (res[i].HasMember("tkfr")) {
                    if (!crypto::str_cmp(res[i]["tkfr"].GetString(), "")) {
                        futureOrder.tkfr = std::stod(res[i]["tkfr"].GetString());
                    }
                }

                if (res[i].HasMember("status")) {
                    futureOrder.status = res[i]["status"].GetString();
                }

                if (res[i].HasMember("finishAs")) {
                    futureOrder.finishAs = res[i]["finishAs"].GetString();
                }
            	
                LOG_INFO("QueryOpenOrder AccountId: {}  GateioPerpetual order: {}", accountInfo.accountId, futureOrder.toString());
                vFutureOrder.emplace_back(futureOrder);
            }
        }

        if (res.HasMember("label")) {
            std::string label = res["label"].GetString();
            std::string msg = "";
            if (label != "USER_NOT_FOUND") {
                query = false;
                if (res.HasMember("message")) {
                    msg = res["message"].GetString();
                } else {
                    msg = label;
                }
            }

            std::string errMsg = fmt::format("GateioPerpetual QueryOpenOrder msg: {}", msg);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        } 
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("GateioPerpetual QueryOpenOrder exception: {}", e.what());
        LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}
