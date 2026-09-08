#include "okx/OkxClient.h"


OkxClient::OkxClient(AccountInfo& info) {
    baseUrl = "https://www.okx.com/";
    accountUrl = "/api/v5/account/balance";
    positionUrl = "/api/v5/account/positions";
    openOrderUrl = "/api/v5/trade/orders-pending";
    accountInfo = info;
}

OkxClient::~OkxClient() {
}

bool OkxClient::QueryAccount(std::vector<okx::OkxAsset>& vAsset, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string ts = crypto::getTimestampIso();
    std::string sign = crypto::getOkxSignatureRest(accountInfo.secretKey, ts, "GET", accountUrl, "");
    std::vector<std::pair<std::string, std::string>> headers = {{"OK-ACCESS-KEY", accountInfo.apiKey}, {"OK-ACCESS-TIMESTAMP", ts}, {"OK-ACCESS-SIGN", sign}, {"OK-ACCESS-PASSPHRASE", accountInfo.passphrase}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), accountUrl, {}, headers, body, status)) {
            std::string errMsg = "Okx QueryAccount syncGet return false";
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("Okx QueryAccount status: {}", status);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        int code = -1;
        if (res.HasMember("code")) {
            code = std::stoi(res["code"].GetString());
        }

        if (code == 0) {
            if (res.HasMember("data") && res["data"].IsArray()) {
                const rapidjson::Value& data = res["data"];
                for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                    if (data[i].HasMember("details") && data[i]["details"].IsArray()) {
                        const rapidjson::Value& details = data[i]["details"];

                        for (rapidjson::SizeType j = 0; j < details.Size(); ++j) {
                            okx::OkxAsset asset;
                            if (details[j].HasMember("ccy")) {
                                asset.ccy = details[j]["ccy"].GetString();
                            }

                            if (details[j].HasMember("eq")) {
                                asset.eq = std::stod(details[j]["eq"].GetString());
                            }

                            if (details[j].HasMember("availableBal")) {
                                asset.availableBal = std::stod(details[j]["availableBal"].GetString());
                            }

                            if (details[j].HasMember("availableEq")) {
                                asset.availableEq = std::stod(details[j]["availableEq"].GetString());
                            }

                            if (details[j].HasMember("cashBal")) {
                                asset.cashBal = std::stod(details[j]["cashBal"].GetString());
                            }

                            if (details[j].HasMember("frozenBal")) {
                                asset.frozenBal = std::stod(details[j]["frozenBal"].GetString());
                            }
       
                            if (details[j].HasMember("liab")) {
                                asset.liab = std::stod(details[j]["liab"].GetString());
                            }


                            if (details[j].HasMember("upl")) {
                                asset.upl = std::stod(details[j]["upl"].GetString());
                            }

   
                            LOG_INFO("QueryAccount AccountId: {}   asset: {}", accountInfo.accountId, asset.toString());
                            vAsset.emplace_back(asset);
                        }
                    }
                }
            }
        }
        else {
            query = false;
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("Okx QueryAccount code: {}, msg: {}", code, msg);
            LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }  
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("Okx QueryAccount exception: {}", e.what());
        LOG_INFO("QueryAccount AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool OkxClient::QueryPosition(std::vector<okx::OkxPosition>& vPosition, std::vector<std::string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string ts = crypto::getTimestampIso();
    std::string sign = crypto::getOkxSignatureRest(accountInfo.secretKey, ts, "GET", positionUrl, "");
    std::vector<std::pair<std::string, std::string>> headers = {{"OK-ACCESS-KEY", accountInfo.apiKey}, {"OK-ACCESS-TIMESTAMP", ts}, {"OK-ACCESS-SIGN", sign}, {"OK-ACCESS-PASSPHRASE", accountInfo.passphrase}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), positionUrl, {}, headers, body, status)) {
            std::string errMsg = "Okx QueryPosition syncGet return false";
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("Okx QueryPosition status: {}", status);
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        int code = -1;
        if (res.HasMember("code")) {
            code = std::stoi(res["code"].GetString());
        }

        if (code == 0) {
            if (res.HasMember("data") && res["data"].IsArray()) {
                const rapidjson::Value& data = res["data"];
                for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                    okx::OkxPosition position;
                    if (data[i].HasMember("instId")) {
                        position.instId = data[i]["instId"].GetString();
                    }

                    if (data[i].HasMember("instType")) {
                        position.instType = data[i]["instType"].GetString();
                    }

                    if (data[i].HasMember("pos")) {
                        position.pos = std::stod(data[i]["pos"].GetString());
                    }

                    if (data[i].HasMember("mmr")) {
                        position.mmr = std::stod(data[i]["mmr"].GetString());
                    }

                    if (data[i].HasMember("avgPx")) {
                        position.avgPx = std::stod(data[i]["avgPx"].GetString());
                    }

                    if (data[i].HasMember("upl")) {
                        position.upl = std::stod(data[i]["upl"].GetString());
                    }
    
                    if (data[i].HasMember("markPx")) {
                        position.markPx = std::stod(data[i]["markPx"].GetString());
                    }

                    if (data[i].HasMember("liqPx")) {
                        position.liqPx = std::stod(data[i]["liqPx"].GetString());
                    }

                    if (data[i].HasMember("adl")) {
                        position.adl = std::stod(data[i]["adl"].GetString());
                    }

                    LOG_INFO("QueryPosition AccountId: {}    position: {}", accountInfo.accountId, position.toString());
                    vPosition.emplace_back(position);
                }
            }
        }
        else {
            query = false;
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("Okx QueryPosition code: {}, msg: {}", code, msg);
            LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }  
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("Okx QueryPosition exception: {}", e.what());
        LOG_INFO("QueryPosition AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}

bool OkxClient::QueryOpenOrder(vector<okx::OkxOrder>& vOpenOrder, vector<string>& vErrorMsg) {
    bool query = true;
    int status = 0;
    std::string body;

    std::string ts = crypto::getTimestampIso();
    std::string sign = crypto::getOkxSignatureRest(accountInfo.secretKey, ts, "GET", openOrderUrl, "");
    std::vector<std::pair<std::string, std::string>> headers = {{"OK-ACCESS-KEY", accountInfo.apiKey}, {"OK-ACCESS-TIMESTAMP", ts}, {"OK-ACCESS-SIGN", sign}, {"OK-ACCESS-PASSPHRASE", accountInfo.passphrase}};

    try {
        if (!Net::Instance().syncGet(accountInfo.accountName, crypto::host_of(accountInfo.restUrl), openOrderUrl, {}, headers, body, status)) {
            std::string errMsg = "Okx QueryOpenOrder syncGet return false";
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }
        if (status != 200) {
            std::string errMsg = fmt::format("Okx QueryOpenOrder status: {}", status);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
            return false;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        int code = -1;
        if (res.HasMember("code")) {
            code = std::stoi(res["code"].GetString());
        }

        if (code == 0) {
            if (res.HasMember("data") && res["data"].IsArray()) {
                const rapidjson::Value& data = res["data"];
                for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                    okx::OkxOrder order;
                    if (data[i].HasMember("instId")) {
                        order.instId = data[i]["instId"].GetString();
                    }

                    if (data[i].HasMember("instType")) {
                        order.instType = data[i]["instType"].GetString();
                    }

                    if (data[i].HasMember("sz")) {
                        order.sz = std::stod(data[i]["sz"].GetString());
                    }

                    if (data[i].HasMember("accFillSz")) {
                        order.accFillSz = std::stod(data[i]["accFillSz"].GetString());
                    }
  
                    if (data[i].HasMember("px")) {
                        order.px = std::stod(data[i]["px"].GetString());
                    }

                    if (data[i].HasMember("avgPx")) {
                        order.avgPx = std::stod(data[i]["avgPx"].GetString());
                    }

                    if (data[i].HasMember("state")) {
                        order.state = data[i]["state"].GetString();
                    }

                    if (data[i].HasMember("side")) {
                        order.side = data[i]["side"].GetString();
                    }

                    if (data[i].HasMember("category")) {
                        order.category = data[i]["category"].GetString();
                    }

                    LOG_INFO("QueryOpenOrder AccountId: {}  order: {}", accountInfo.accountId, order.toString());
                    vOpenOrder.emplace_back(order);
                }
            }
        }
        else {
            query = false;
            std::string msg = "";
            if (res.HasMember("msg")) {
                msg = res["msg"].GetString();
            }

            std::string errMsg = fmt::format("Okx QueryOpenOrder code: {}, msg: {}", code, msg);
            LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
            vErrorMsg.emplace_back(errMsg);
        }  
    }
    catch(exception& e) {
        query = false;
        std::string errMsg = fmt::format("Okx QueryOpenOrder exception: {}", e.what());
        LOG_INFO("QueryOpenOrder AccountId: {} Error: {}", accountInfo.accountId, errMsg);
        vErrorMsg.emplace_back(errMsg);
    }

    return query;
}