#include "okx/OkxClient.h"


OkxClient::OkxClient(AccountInfo& info) {
    baseUrl = "https://www.okx.com/";
    accountUrl = "/api/v5/account/balance";
    positionUrl = "/api/v5/account/positions";
    openOrderUrl = "/api/v5/trade/orders-pending";
    historyOrderUrl = "/api/v5/trade/orders-history";
    accountInfo = info;
}

OkxClient::~OkxClient() {
}

bool OkxClient::QueryAccount(vector<okx::OkxAsset>& vAsset, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vAsset.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            string time = GetTimestamp();
            string sign = get_signature_rest(time, "GET", accountUrl.to_string(), "");
            request.headers().add("OK-ACCESS-KEY", accountInfo.apiKey);
            request.headers().add("OK-ACCESS-TIMESTAMP", time);
            request.headers().add("OK-ACCESS-SIGN", sign);
            request.headers().add("OK-ACCESS-PASSPHRASE", accountInfo.passphrase);
            uri_builder builder(accountUrl);

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

                LOG_INFO("QueryAccount AccountId: %d  OkxClient response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const &v = previousTask.get();
                //LOG_INFO("get_balance: %s", v.serialize().c_str());
                if(v.has_field("data") && v.at("code").as_string()[0] == '0') {
                    auto array = v.at("data").at(0).at("details").as_array();
                    for(auto &it : array) {
                        okx::OkxAsset asset;
                        if (it.has_field("ccy")) {
                            asset.ccy = it.at("ccy").as_string();
                        }

                        if (it.has_field("eq")) {
                            string eq = it.at("eq").as_string();
                            if (eq != "") {
                                asset.eq = stod(eq);
                            }
                        }

                        if (it.has_field("availableBal")) {
                            string availableBal = it.at("availableBal").as_string();
                            if (availableBal != "") {
                                asset.availableBal = stod(availableBal);
                            }
                        }

                        if (it.has_field("availableEq")) {
                            string availableEq = it.at("availableEq").as_string();
                            if (availableEq != "") {
                                asset.availableEq = stod(availableEq);
                            }
                        }

                        if (it.has_field("cashBal")) {
                            string cashBal = it.at("cashBal").as_string();
                            if (cashBal != "") {
                                asset.cashBal = stod(cashBal);
                            }
                        }

                        if (it.has_field("frozenBal")) {
                            string frozenBal = it.at("frozenBal").as_string();
                            if (frozenBal != "") {
                                asset.frozenBal = stod(frozenBal);
                            }
                        }

                        if (it.has_field("liab")) {
                            string liab = it.at("liab").as_string();
                            if (liab != "") {
                                asset.liab = stod(liab);
                            }
                        }

                        if (it.has_field("upl")) {
                            string upl = it.at("upl").as_string();
                            if (upl != "") {
                                asset.upl = stod(upl);
                            }
                        }
                        LOG_INFO("QueryAccount AccountId: %d    uFutureAsset: %s", accountInfo.accountId, asset.toString().c_str());
                        vAsset.emplace_back(asset);
                    }
                }

                if(v.has_field("data") && v.at("code").as_string()[0] != '0') {
                    query = false;
                    stringstream ss;
                    int code = stod(v.at("code").as_string());
                    string msg = "";
                    if (v.has_field("msg")) {
                        msg = v.at("msg").as_string();
                    }
                    ss << "Okx QueryAccount code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   Okx Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
                
            })
            .wait();
        }
        catch(exception& e) {
            query = false;
            string errMsg = string("Okx QueryAccount ") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   Okx Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

bool OkxClient::QueryPosition(vector<okx::OkxPosition>& vPosition, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vPosition.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            string time = GetTimestamp();
            string sign = get_signature_rest(time, "GET", positionUrl.to_string(), "");
            request.headers().add("OK-ACCESS-KEY", accountInfo.apiKey);
            request.headers().add("OK-ACCESS-TIMESTAMP", time);
            request.headers().add("OK-ACCESS-SIGN", sign);
            request.headers().add("OK-ACCESS-PASSPHRASE", accountInfo.passphrase);
            uri_builder builder(positionUrl);

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

                LOG_INFO("QueryPosition AccountId: %d  OkxClient response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const &v = previousTask.get();
                //LOG_INFO("get position: %s", v.serialize().c_str());
                if(v.has_field("data") && v.at("code").as_string()[0] == '0'){
                    auto array = v.at("data").as_array();
                    for(auto &it : array) {
                        okx::OkxPosition position;
                        if (it.has_field("instId")) {
                            position.instId = it.at("instId").as_string();
                        }

                        if (it.has_field("instType")) {
                            position.instType = it.at("instType").as_string();
                        }

                        if (it.has_field("pos")) {
                            string pos = it.at("pos").as_string();
                            if (pos != "") {
                                position.pos = stod(pos);
                            }
                        }

                        if (it.has_field("mmr")) {
                            string mmr = it.at("mmr").as_string();
                            if (mmr != "") {
                                position.mmr = stod(mmr);
                            }
                        }

                        if (it.has_field("avgPx")) {
                            string avgPx = it.at("avgPx").as_string();
                            if (avgPx != "") {
                                position.avgPx = stod(avgPx);
                            }
                        }

                        if (it.has_field("upl")) {
                            string upl = it.at("upl").as_string();
                            if (upl != "") {
                                position.upl = stod(upl);
                            }
                        }

                        if (it.has_field("markPx")) {
                            string markPx = it.at("markPx").as_string();
                            if (markPx != "") {
                                position.markPx = stod(markPx);
                            }
                        }

                        if (it.has_field("liqPx")) {
                            string liqPx = it.at("liqPx").as_string();
                            if (liqPx != "") {
                                position.liqPx = stod(liqPx);
                            }
                        }

                        if (it.has_field("adl")) {
                            string adl = it.at("adl").as_string();
                            if (adl != "") {
                                position.adl = stod(adl);
                            }
                        }

                        LOG_INFO("QueryPosition AccountId: %d    position: %s", accountInfo.accountId, position.toString().c_str());
                        vPosition.emplace_back(position);
                    }
                }

                if(v.has_field("data") && v.at("code").as_string()[0] != '0') {
                    query = false;
                    stringstream ss;
                    int code = stod(v.at("code").as_string());
                    string msg = "";
                    if (v.has_field("msg")) {
                        msg = v.at("msg").as_string();
                    }
                    ss << "Okx QueryPosition code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryPosition AccountId: %d   Okx Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
                
            })
            .wait();
        }
        catch(exception& e) {
            query = false;
            string errMsg = string("Okx QueryPosition ") + string(e.what());
            LOG_INFO("QueryPosition AccountId: %d   Okx Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

bool OkxClient::QueryOpenOrder(vector<okx::OkxOrder>& vOpenOrder, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vOpenOrder.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            string time = GetTimestamp();
            string sign = get_signature_rest(time, "GET", openOrderUrl.to_string(), "");
            request.headers().add("OK-ACCESS-KEY", accountInfo.apiKey);
            request.headers().add("OK-ACCESS-TIMESTAMP", time);
            request.headers().add("OK-ACCESS-SIGN", sign);
            request.headers().add("OK-ACCESS-PASSPHRASE", accountInfo.passphrase);
            uri_builder builder(openOrderUrl);

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

                LOG_INFO("QueryOpenOrder AccountId: %d  OkxClient response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const &v = previousTask.get();
                //LOG_INFO("get open order: %s", v.serialize().c_str());
                if(v.has_field("data") && v.at("code").as_string()[0] == '0'){
                    auto array = v.at("data").as_array();
                    for(auto &it : array) {
                        okx::OkxOrder order;
                        if (it.has_field("instId")) {
                            order.instId = it.at("instId").as_string();
                        }

                        if (it.has_field("instType")) {
                            order.instType = it.at("instType").as_string();
                        }

                        if (it.has_field("sz")) {
                            string sz = it.at("sz").as_string();
                            if (sz != "") {
                                order.sz = stod(sz);
                            }
                        }

                        if (it.has_field("accFillSz")) {
                            string accFillSz = it.at("accFillSz").as_string();
                            if (accFillSz != "") {
                                order.accFillSz = stod(accFillSz);
                            }
                        }

                        if (it.has_field("px")) {
                            string px = it.at("px").as_string();
                            if (px != "") {
                                order.px = stod(px);
                            }
                        }

                        if (it.has_field("avgPx")) {
                            string avgPx = it.at("avgPx").as_string();
                            if (avgPx != "") {
                                order.avgPx = stod(avgPx);
                            }
                        }

                        if (it.has_field("state")) {
                            order.state = it.at("state").as_string();
                        }

                        if (it.has_field("side")) {
                            order.side = it.at("side").as_string();
                        }

                        if (it.has_field("category")) {
                            order.category = it.at("category").as_string();
                        }

                        LOG_INFO("QueryOpenOrder AccountId: %d    order: %s", accountInfo.accountId, order.toString().c_str());
                        vOpenOrder.emplace_back(order);
                    }
                }

                if(v.has_field("data") && v.at("code").as_string()[0] != '0') {
                    query = false;
                    stringstream ss;
                    int code = stod(v.at("code").as_string());
                    string msg = "";
                    if (v.has_field("msg")) {
                        msg = v.at("msg").as_string();
                    }
                    ss << "Okx QueryOpenOrder code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryOpenOrder AccountId: %d   Okx Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
                
            })
            .wait();
        }
        catch(exception& e) {
            query = false;
            string errMsg = string("Okx QueryOpenOrder ") + string(e.what());
            LOG_INFO("QueryOpenOrder AccountId: %d   Okx Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

bool OkxClient::QueryOrder(vector<okx::OkxOrder>& vOrder, vector<string>& vErrorMsg) {
    bool query = true;
    vector<okx::OkxOrder> vSpotOrder;
    vector<string> vSpotErr;
    bool spotQuery = QueryOrder(vSpotOrder, vSpotErr, "SPOT");
    vOrder.insert(vOrder.end(), vSpotOrder.begin(), vSpotOrder.end());
    vErrorMsg.insert(vErrorMsg.end(), vSpotErr.begin(), vSpotErr.end());
    query = query && spotQuery;

    usleep(10);
    vector<okx::OkxOrder> vSwapOrder;
    vector<string> vSwapErr;
    bool swapQuery = QueryOrder(vSwapOrder, vSwapErr, "SWAP");
    vOrder.insert(vOrder.end(), vSwapOrder.begin(), vSwapOrder.end());
    vErrorMsg.insert(vErrorMsg.end(), vSwapErr.begin(), vSwapErr.end());
    query = query&& swapQuery;

    return query;
}

bool OkxClient::QueryOrder(vector<okx::OkxOrder>& vOrder, vector<string>& vErrorMsg, string instType) {
    int count = 0;
    bool query = true;
    while (count < 3) {
        query = true;
        vOrder.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            string time = GetTimestamp();

            int64_t oneMinute = 60 * 1000;
            int64_t currentTimeMs = gettickcount();
            int64_t beginTime = currentTimeMs - 60 * oneMinute;
            string beginTimeStr = to_string(beginTime);

            string queryStr{"?instType="};
            queryStr.append(instType);
            // queryStr.append(instType).append("&");
            // queryStr.append("begin=").append(beginTimeStr);

            string path = historyOrderUrl.to_string() + queryStr;
            uri_builder builder(path.c_str());
            string sign = get_signature_rest(time, "GET", builder.to_string(), "");

            request.headers().add("OK-ACCESS-KEY", accountInfo.apiKey);
            request.headers().add("OK-ACCESS-TIMESTAMP", time);
            request.headers().add("OK-ACCESS-SIGN", sign);
            request.headers().add("OK-ACCESS-PASSPHRASE", accountInfo.passphrase);

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

                LOG_INFO("QueryOrder AccountId: %d  OkxClient response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const &v = previousTask.get();
                //LOG_INFO("get open order: %s", v.serialize().c_str());
                if(v.has_field("data") && v.at("code").as_string()[0] == '0'){
                    auto array = v.at("data").as_array();
                    for(auto &it : array) {
                        okx::OkxOrder order;
                        if (it.has_field("instId")) {
                            order.instId = it.at("instId").as_string();
                        }

                        if (it.has_field("instType")) {
                            order.instType = it.at("instType").as_string();
                        }

                        if (it.has_field("sz")) {
                            string sz = it.at("sz").as_string();
                            if (sz != "") {
                                order.sz = stod(sz);
                            }
                        }

                        if (it.has_field("accFillSz")) {
                            string accFillSz = it.at("accFillSz").as_string();
                            if (accFillSz != "") {
                                order.accFillSz = stod(accFillSz);
                            }
                        }

                        if (it.has_field("px")) {
                            string px = it.at("px").as_string();
                            if (px != "") {
                                order.px = stod(px);
                            }
                        }

                        if (it.has_field("avgPx")) {
                            string avgPx = it.at("avgPx").as_string();
                            if (avgPx != "") {
                                order.avgPx = stod(avgPx);
                            }
                        }

                        if (it.has_field("state")) {
                            order.state = it.at("state").as_string();
                        }

                        if (it.has_field("side")) {
                            order.side = it.at("side").as_string();
                        }

                        if (it.has_field("category")) {
                            order.category = it.at("category").as_string();
                        }

                        if (it.has_field("uTime")) {
                            order.updateTime = stoll(it.at("uTime").as_string());
                        }

                        if (it.has_field("cTime")) {
                            order.createTime = stoll(it.at("cTime").as_string());
                        }

                        LOG_INFO("QueryOrder AccountId: %d    order: %s", accountInfo.accountId, order.toString().c_str());

                        if (order.category == "twap" || order.category == "adl") {
                            vOrder.emplace_back(order);
                        } 
                    }
                }

                if(v.has_field("data") && v.at("code").as_string()[0] != '0') {
                    query = false;
                    stringstream ss;
                    int code = stod(v.at("code").as_string());
                    string msg = "";
                    if (v.has_field("msg")) {
                        msg = v.at("msg").as_string();
                    }
                    ss << "Okx QueryOrder code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryOrder AccountId: %d   Okx Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }
                
            })
            .wait();
        }
        catch(exception& e) {
            query = false;
            string errMsg = string("Okx QueryOrder ") + string(e.what());
            LOG_INFO("QueryOrder AccountId: %d   Okx Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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

std::string OkxClient::base64_encode(unsigned char const * input, size_t len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (len--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++) {
                ret += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) {
            ret += base64_chars[char_array_4[j]];
        }

        while((i++ < 3)) {
            ret += '=';
        }
    }

    return ret;
}

string OkxClient::base64_encode(std::string const & input) {
    return base64_encode(
            reinterpret_cast<const unsigned char *>(input.data()),
            input.size()
    );
}

int OkxClient::HmacEncode(const char * algo,
               const char * key, unsigned int key_length,
               const char * input, unsigned int input_length,
               unsigned char * &output, unsigned int &output_length) {
    const EVP_MD * engine = NULL;
    if(strcasecmp("sha512", algo) == 0) {
        engine = EVP_sha512();
    }
    else if(strcasecmp("sha256", algo) == 0) {
        engine = EVP_sha256();
    }
    else if(strcasecmp("sha1", algo) == 0) {
        engine = EVP_sha1();
    }
    else if(strcasecmp("md5", algo) == 0) {
        engine = EVP_md5();
    }
    else if(strcasecmp("sha224", algo) == 0) {
        engine = EVP_sha224();
    }
    else if(strcasecmp("sha384", algo) == 0) {
        engine = EVP_sha384();
    }
    else if(strcasecmp("sha", algo) == 0) {
        //engine = EVP_sha();
        assert(0);
    }
    else {
        cout << "Algorithm " << algo << " is not supported by this program!" << endl;
        return -1;
    }

    output = (unsigned char*)malloc(EVP_MAX_MD_SIZE);

    /*
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, strlen(key), engine, NULL);
    HMAC_Update(&ctx, (unsigned char*)input, strlen(input));        // input is OK; &input is WRONG !!!
    HMAC_Final(&ctx, output, &output_length);
    HMAC_CTX_cleanup(&ctx);
    */
    return 0;
}

string OkxClient::getSignature(const string &query,const string &apiSecret) {
    unsigned char * mac = NULL;
    unsigned int mac_length = 0;
    int ret = HmacEncode("sha256", apiSecret.c_str(), apiSecret.length(), query.c_str(), query.length(), mac, mac_length);
    string signature = base64_encode(mac, mac_length);
    return signature;
}

string OkxClient::get_signature_rest(const string &timestamp, const string &method,
            const string &requestPath,const string &body){

    unsigned char * mac = NULL;
    unsigned int mac_length = 0;
    string data = timestamp + method + requestPath + body;
    string key = accountInfo.secretKey;   
    string sign = getSignature(data, key);
    return sign;
}
