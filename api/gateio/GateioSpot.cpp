#include "GateioSpot.h"


GateioSpot::GateioSpot(AccountInfo& info) {
    accountUrl = "/api/v4/spot/accounts";
    accountInfo = info;
}

GateioSpot::~GateioSpot() {
}

bool GateioSpot::QueryAccount(vector<gateio::SpotAsset>& vSpotAsset, vector<string>& vErrorMsg) {
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
            uri_builder builder(accountUrl);
            
            string hashStr = sha512("");
            int64_t timestamp = gettickcount() / 1000;
            stringstream ss;
            ss << "GET" << "\n" << accountUrl << "\n" << "" << "\n" << hashStr << "\n" << timestamp;
            string message = ss.str();
            string signature = getSignatureGate(message, accountInfo.secretKey);
            request.headers().add("Accept", "application/json");  
            request.headers().add("Content-Type", "application/json");  
            request.headers().add("SIGN", signature);  
            request.headers().add("Timestamp", timestamp);
            request.headers().add("KEY", accountInfo.apiKey);

            request.set_request_uri(builder.to_string());
            
            client.request(request)
            .then([](http_response response) -> pplx::task<json::value> {  // if the status is OK extract the body of the response into a JSON value
                auto code = response.status_code();
                if(code == status_codes::OK || code == status_codes::BadRequest || code == status_codes::NotFound) {  // || code == status_codes::TooManyRequests || code == status_codes::Unauthorized
                    return response.extract_json();
                }
                
                LOG_INFO("GateioSpot QueryAccount response: '%s' ", response.to_string().c_str());
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.is_array()) {
                    auto& assetArray = content.as_array();
                    for (auto& asset : assetArray) {
                        gateio::SpotAsset spotAsset;
                        if (asset.has_field("currency")) {
                            string currency = asset.at("currency").as_string();
                            spotAsset.currency = boost::to_upper_copy(currency);
                        }
                        if (asset.has_field("available")) {
                            spotAsset.available = fabs(stod(asset.at("available").as_string()));
                        }
                        if (asset.has_field("locked")) {
                            spotAsset.locked = fabs(stod(asset.at("locked").as_string()));
                        }

                        if ((spotAsset.available + spotAsset.locked) >= 0.0000000001) {
                            spotAsset.total = spotAsset.available + spotAsset.locked;
                    	    LOG_INFO("QueryAccount AccountId: %d   GateioSpot asset: %s", accountInfo.accountId, spotAsset.toString().c_str());
                            vSpotAsset.emplace_back(spotAsset);
                        }
                    }
                }

                if (content.has_field("label")) {
                    query = false;
                    stringstream ss;
                    string label = content.at("label").as_string();
                    string msg = "";
                    if (label.size() > 0) {
                        if (content.has_field("message")) {
                            msg = content.at("message").as_string();
                        } else {
                            msg = content.at("label").as_string();
                        }
                    }
                    ss << "GateioSpot QueryAccount msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryAccount AccountId: %d   GateioSpot Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }      

            })
            .wait();
        } catch(exception& e) {
            query = false;
            string errMsg = string("GateioSpot QueryAccount") + string(e.what());
            LOG_INFO("QueryAccount AccountId: %d   GateioSpot Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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
