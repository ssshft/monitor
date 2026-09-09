#include "bybit/Bybit.h"
#include "Net.h"


Bybit::Bybit(AccountInfo& info) {
    accountUrl = "/v5/account/wallet-balance";
    positionUrl = "/v5/position/list";
    orderUrl = "/v5/order/realtime";
    accountInfo = info;
}

Bybit::~Bybit() {
}

bool Bybit::QueryAccount(bybit::TotalAccountInfo& totalAccountInfo, std::vector<bybit::Asset>& vAsset, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string queryStr = "accountType=UNIFIED";
    std::string fullPath = accountUrl + "?" + queryStr;

    std::string ts = std::to_string(crypto::getCurrentTimeMilli());
    std::string sign = crypto::getBybitSignatureRest(accountInfo.secretKey, ts, accountInfo.apiKey, "5000", queryStr);
    std::vector<std::pair<std::string, std::string>> headers = {{"X-BAPI-API-KEY", accountInfo.apiKey}, {"X-BAPI-TIMESTAMP", ts}, {"X-BAPI-RECV-WINDOW", "5000"}, {"X-BAPI-SIGN", sign}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), fullPath, {}, headers, body, status)) {
            std::string errMsg = "Bybit QueryAccount syncGet return false";
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("Bybit QueryAccount status: {}", status);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.HasMember("result")) {
            const rapidjson::Value& result = res["result"];
            if (result.HasMember("list") && result["list"].IsArray()) {
                const rapidjson::Value& data = res["list"];
                for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                    if (data[i].HasMember("accountType")) {
                        totalAccountInfo.accountType = data[i]["accountType"].GetString();
                    }

                    if (data[i].HasMember("totalEquity")) {
                        if (!crypto::str_cmp(data[i]["totalEquity"].GetString(), "")) {
                            totalAccountInfo.accountType = std::stod(data[i]["totalEquity"].GetString());
                        }   
                    }

                    if (data[i].HasMember("accountIMRate")) {
                        if (!crypto::str_cmp(data[i]["accountIMRate"].GetString(), "")) {
                            totalAccountInfo.accountIMRate = std::stod(data[i]["accountIMRate"].GetString());
                        }   
                    }

                    if (data[i].HasMember("totalMarginBalance")) {
                        if (!crypto::str_cmp(data[i]["totalMarginBalance"].GetString(), "")) {
                            totalAccountInfo.totalMarginBalance = std::stod(data[i]["totalMarginBalance"].GetString());
                        }   
                    }
 
                    if (data[i].HasMember("totalInitialMargin")) {
                        if (!crypto::str_cmp(data[i]["totalInitialMargin"].GetString(), "")) {
                            totalAccountInfo.totalInitialMargin = std::stod(data[i]["totalInitialMargin"].GetString());
                        }   
                    }
      
                    if (data[i].HasMember("totalAvailableBalance")) {
                        if (!crypto::str_cmp(data[i]["totalAvailableBalance"].GetString(), "")) {
                            totalAccountInfo.totalAvailableBalance = std::stod(data[i]["totalAvailableBalance"].GetString());
                        }   
                    }
     
                    if (data[i].HasMember("accountMMRate")) {
                        if (!crypto::str_cmp(data[i]["accountMMRate"].GetString(), "")) {
                            totalAccountInfo.accountMMRate = std::stod(data[i]["accountMMRate"].GetString());
                        }   
                    }
       
                    if (data[i].HasMember("totalPerpUPL")) {
                        if (!crypto::str_cmp(data[i]["totalPerpUPL"].GetString(), "")) {
                            totalAccountInfo.totalPerpUPL = std::stod(data[i]["totalPerpUPL"].GetString());
                        }   
                    }
                
                    if (data[i].HasMember("totalWalletBalance")) {
                        if (!crypto::str_cmp(data[i]["totalWalletBalance"].GetString(), "")) {
                            totalAccountInfo.totalWalletBalance = std::stod(data[i]["totalWalletBalance"].GetString());
                        }   
                    }
                
                    if (data[i].HasMember("accountLTV")) {
                        if (!crypto::str_cmp(data[i]["accountLTV"].GetString(), "")) {
                            totalAccountInfo.accountLTV = std::stod(data[i]["accountLTV"].GetString());
                        }   
                    }
       
                    if (data[i].HasMember("totalMaintenanceMargin")) {
                        if (!crypto::str_cmp(data[i]["totalMaintenanceMargin"].GetString(), "")) {
                            totalAccountInfo.totalMaintenanceMargin = std::stod(data[i]["totalMaintenanceMargin"].GetString());
                        }   
                    }
 
                    LOG_INFO("QueryAccount AccountId: {}   Bybit totalAccountInfo: {}", accountInfo.accountId, totalAccountInfo.toString());

                    if (data[i].HasMember("coin") && data[i]["coin"].IsArray()) {
                        const rapidjson::Value& coinArr = data[i]["coin"];
                        for (rapidjson::SizeType j = 0; j < coinArr.Size(); ++j) {
                            bybit::Asset asset;
                            if (coinArr[j].HasMember("coin")) {
                                asset.coin = coinArr[j]["coin"].GetString();
                            }

                            if (coinArr[j].HasMember("availableToBorrow")) {
                                asset.availableToBorrow = std::stod(coinArr[j]["availableToBorrow"].GetString());
                            }
     
                            if (coinArr[j].HasMember("bonus")) {
                                asset.bonus = std::stod(coinArr[j]["bonus"].GetString());
                            }

                            if (coinArr[j].HasMember("accruedInterest")) {
                                asset.accruedInterest = std::stod(coinArr[j]["accruedInterest"].GetString());
                            }

                            if (coinArr[j].HasMember("availableToWithdraw")) {
                                asset.availableToWithdraw = std::stod(coinArr[j]["availableToWithdraw"].GetString());
                            }

                            if (coinArr[j].HasMember("totalOrderIM")) {
                                asset.totalOrderIM = std::stod(coinArr[j]["totalOrderIM"].GetString());
                            }            

                            if (coinArr[j].HasMember("equity")) {
                                asset.equity = std::stod(coinArr[j]["equity"].GetString());
                            }  

                            if (coinArr[j].HasMember("totalPositionMM")) {
                                asset.totalPositionMM = std::stod(coinArr[j]["totalPositionMM"].GetString());
                            }  

                            if (coinArr[j].HasMember("usdValue")) {
                                asset.usdValue = std::stod(coinArr[j]["usdValue"].GetString());
                            }   

                            if (coinArr[j].HasMember("unrealisedPnl")) {
                                asset.unrealisedPnl = std::stod(coinArr[j]["unrealisedPnl"].GetString());
                            }                      

                            if (coinArr[j].HasMember("borrowAmount")) {
                                asset.borrowAmount = std::stod(coinArr[j]["borrowAmount"].GetString());
                            }                 

                            if (coinArr[j].HasMember("totalPositionIM")) {
                                asset.totalPositionIM = std::stod(coinArr[j]["totalPositionIM"].GetString());
                            }   

                            if (coinArr[j].HasMember("walletBalance")) {
                                asset.walletBalance = std::stod(coinArr[j]["walletBalance"].GetString());
                            }   

                            if (coinArr[j].HasMember("cumRealisedPnl")) {
                                asset.cumRealisedPnl = std::stod(coinArr[j]["cumRealisedPnl"].GetString());
                            }   

                            if (fabs(asset.walletBalance) >= 0.0000000001) {
                                LOG_INFO("QueryAccount AccountId: {}   Bybit asset: {}", accountInfo.accountId, asset.toString());
                                vAsset.emplace_back(asset);       
                            }
                        }
                    }
                }
            }
        }

        if (res.HasMember("retCode")) {
            int code = std::stoi(res["retCode"].GetString());
            if (code != 0) {
                query = false;
                std::string msg = "";
                if (res.HasMember("retMsg")) {
                    msg = res["retMsg"].GetString();
                }

                std::string errMsg = fmt::format("Bybit QueryAccount code: {}, msg: {}", code, msg);
                LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
                vErrorMsg.emplace_back(errMsg);
            }
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("Bybit QueryAccount exception: {}", e.what());
        LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool Bybit::QueryPosition(vector<bybit::Position>& vPosition, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string queryStr = fmt::format("category={}&settleCoin=USDT", "linear");
    std::string fullPath = positionUrl + "?" + queryStr;

    std::string ts = std::to_string(crypto::getCurrentTimeMilli());
    std::string sign = crypto::getBybitSignatureRest(accountInfo.secretKey, ts, accountInfo.apiKey, "5000", queryStr);
    std::vector<std::pair<std::string, std::string>> headers = {{"X-BAPI-API-KEY", accountInfo.apiKey}, {"X-BAPI-TIMESTAMP", ts}, {"X-BAPI-RECV-WINDOW", "5000"}, {"X-BAPI-SIGN", sign}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), fullPath, {}, headers, body, status)) {
            std::string errMsg = "Bybit QueryPosition syncGet return false";
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("Bybit QueryPosition status: {}", status);
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.HasMember("result")) {
            const rapidjson::Value& result = res["result"];
            if (result.HasMember("list") && result["list"].IsArray()) {
                const rapidjson::Value& data = res["list"];
                for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                    bybit::Position position;
                    if (data[i].HasMember("symbol")) {
                        position.symbol = data[i]["symbol"].GetString();
                    }

                    if (data[i].HasMember("size")) {
                        position.size = std::stod(data[i]["size"].GetString());
                    }

                    if (data[i].HasMember("side")) {
                        position.side = data[i]["side"].GetString();
                    }

                    if (data[i].HasMember("category")) {
                        position.category = data[i]["category"].GetString();
                    }

                    if (data[i].HasMember("leverage")) {
                        position.leverage = std::stod(data[i]["leverage"].GetString());
                    }

                    if (data[i].HasMember("avgPrice")) {
                        position.avgPrice = std::stod(data[i]["avgPrice"].GetString());
                    }

                    if (data[i].HasMember("liqPrice")) {
                        position.liqPrice = std::stod(data[i]["liqPrice"].GetString());
                    }

                    if (data[i].HasMember("takeProfit")) {
                        position.takeProfit = std::stod(data[i]["takeProfit"].GetString());
                    }
   
                    if (data[i].HasMember("unrealisedPnl")) {
                        position.unrealisedPnl = std::stod(data[i]["unrealisedPnl"].GetString());
                    }

                    if (data[i].HasMember("markPrice")) {
                        position.markPrice = std::stod(data[i]["markPrice"].GetString());
                    }

                    if (data[i].HasMember("cumRealisedPnl")) {
                        position.cumRealisedPnl = std::stod(data[i]["cumRealisedPnl"].GetString());
                    }

                    if (data[i].HasMember("positionMM")) {
                        position.positionMM = std::stod(data[i]["positionMM"].GetString());
                    }

                    if (fabs(position.size) > 0.0000000001) {
                        LOG_INFO("QueryPosition AccountId: {}   Bybit position: {}", accountInfo.accountId, position.toString());
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

        if (res.HasMember("retCode")) {
            int code = std::stoi(res["retCode"].GetString());
            if (code != 0) {
                query = false;
                std::string msg = "";
                if (res.HasMember("retMsg")) {
                    msg = res["retMsg"].GetString();
                }

                std::string errMsg = fmt::format("Bybit QueryPosition code: {}, msg: {}", code, msg);
                LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
                vErrorMsg.emplace_back(errMsg);
            }
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("Bybit QueryPosition exception: {}", e.what());
        LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool Bybit::QueryOpenOrder(vector<bybit::Order>& vOpenOrder, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string queryStr = "category=linear&settleCoin=USDT&orderFilter=Order&limit=50";
    std::string fullPath = orderUrl + "?" + queryStr;

    std::string ts = std::to_string(crypto::getCurrentTimeMilli());
    std::string sign = crypto::getBybitSignatureRest(accountInfo.secretKey, ts, accountInfo.apiKey, "5000", queryStr);
    std::vector<std::pair<std::string, std::string>> headers = {{"X-BAPI-API-KEY", accountInfo.apiKey}, {"X-BAPI-TIMESTAMP", ts}, {"X-BAPI-RECV-WINDOW", "5000"}, {"X-BAPI-SIGN", sign}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), fullPath, {}, headers, body, status)) {
            std::string errMsg = "Bybit QueryOpenOrder syncGet return false";
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("Bybit QueryOpenOrder status: {}", status);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        if (res.HasMember("result")) {
            const rapidjson::Value& result = res["result"];
            if (result.HasMember("list") && result["list"].IsArray()) {
                const rapidjson::Value& data = res["list"];
                for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                    bybit::Order order;
                    if (data[i].HasMember("orderId")) {
                        order.orderId = data[i]["orderId"].GetString();
                    }

                    if (data[i].HasMember("orderLinkId")) {
                        order.orderLinkId = data[i]["orderLinkId"].GetString();
                    }

                    if (data[i].HasMember("symbol")) {
                        order.symbol = data[i]["symbol"].GetString();
                    }
   
                    if (data[i].HasMember("price")) {
                        order.price = std::stod(data[i]["price"].GetString());
                    }  

                    if (data[i].HasMember("qty")) {
                        order.qty = std::stod(data[i]["qty"].GetString());
                    }  

                    if (data[i].HasMember("side")) {
                        order.side = data[i]["side"].GetString();
                    }             

                    if (data[i].HasMember("avgPrice")) {
                        order.avgPrice = std::stod(data[i]["avgPrice"].GetString());
                    } 

                    if (data[i].HasMember("cumExecQty")) {
                        order.cumExecQty = std::stod(data[i]["cumExecQty"].GetString());
                    } 

                    if (data[i].HasMember("cumExecValue")) {
                        order.cumExecValue = std::stod(data[i]["cumExecValue"].GetString());
                    } 

                    if (data[i].HasMember("cumExecFee")) {
                        order.cumExecFee = std::stod(data[i]["cumExecFee"].GetString());
                    } 

                    if (data[i].HasMember("timeInForce")) {
                        order.timeInForce = data[i]["timeInForce"].GetString();
                    }  

                    if (data[i].HasMember("orderType")) {
                        order.orderType = data[i]["orderType"].GetString();
                    }  

                    if (data[i].HasMember("createdTime")) {
                        order.createdTime = std::stoll(data[i]["createdTime"].GetString());
                    } 

                    if (data[i].HasMember("updatedTime")) {
                        order.updatedTime = std::stoll(data[i]["updatedTime"].GetString());
                    } 
 
                    if (fabs(order.qty) > 0.0000000001) {
                        LOG_INFO("QueryOpenOrder AccountId: {}  Bybit Order: {}", accountInfo.accountId, order.toString());
                        vOpenOrder.emplace_back(order);
                    }
                }
            }
        }

        if (res.HasMember("retCode")) {
            int code = std::stoi(res["retCode"].GetString());
            if (code != 0) {
                query = false;
                std::string msg = "";
                if (res.HasMember("retMsg")) {
                    msg = res["retMsg"].GetString();
                }

                std::string errMsg = fmt::format("Bybit QueryOpenOrder code: {}, msg: {}", code, msg);
                LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
                vErrorMsg.emplace_back(errMsg);
            }
        }
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("Bybit QueryOpenOrder exception: {}", e.what());
        LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}