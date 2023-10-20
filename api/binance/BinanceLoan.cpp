#include "BinanceLoan.h"


BinanceLoan::BinanceLoan(AccountInfo& info) {
    loadBorrowUrl = "/sapi/v1/loan/ongoing/orders";
    accountInfo = info;
}

BinanceLoan::~BinanceLoan() {
}

bool BinanceLoan::QueryLoanBorrow(vector<binance::LoanBorrow>& vLoanBorrow, vector<string>& vErrorMsg) {
    int count = 0;
    bool query = true;
    while (count < 1) {
        query = true;
        vLoanBorrow.clear();
        vErrorMsg.clear();
        try {
            http_client_config config;
            config.set_timeout(utility::seconds(5));
            http_client client(accountInfo.restUrl, config);
            http_request request(methods::GET);
            request.headers().add("X-MBX-APIKEY", accountInfo.apiKey);
            uri_builder builder(loadBorrowUrl);

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

                LOG_INFO("QueryLoanBorrow AccountId: %d  BinanceLoan response: '%s' ", accountInfo.accountId, response.to_string().c_str());
                // throw IGException(response.to_string().c_str(), code);
                throw exception();
                return pplx::task_from_result(json::value());  // return an empty JSON value
            })
            .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
                json::value const& content = previousTask.get();
                if (content.has_field("rows")) {
                    json::value const& rows = content.at("rows");
                    if (rows.is_array()) {
                        auto loanArray = rows.as_array();
                        for (auto& loan : loanArray) {
                            binance::LoanBorrow loanBorrow;
                            if (loan.has_field("loanCoin")) {
                                loanBorrow.loanCoin = loan.at("loanCoin").as_string();
                            }
                            if (loan.has_field("totalDebt")) {
                                loanBorrow.loanAmount = stod(loan.at("totalDebt").as_string());
                            }
                            if (loan.has_field("collateralCoin")) {
                                loanBorrow.collateralCoin = loan.at("collateralCoin").as_string();
                            }
                            if (loan.has_field("collateralAmount")) {
                                loanBorrow.collateralAmount = stod(loan.at("collateralAmount").as_string());
                            }
                        
                            LOG_INFO("QueryLoanBorrow AccountId: %d    loanBorrow: %s", accountInfo.accountId, loanBorrow.toString().c_str());
                            vLoanBorrow.emplace_back(loanBorrow);
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
                    ss << "BinanceLoan QueryLoanBorrow code:" << code << " msg:" << msg;
                    string errMsg = ss.str();
                    LOG_INFO("QueryLoanBorrow AccountId: %d   BinanceLoan Error: '%s' ", accountInfo.accountId, errMsg.c_str());
                    vErrorMsg.emplace_back(errMsg);
                }

            })
            .wait();
        }
        /*
        catch(IGException& e) {
            query = false;
            stringstream ss;
            ss << "BinanceLoan QueryLoanBorrow code:" << e.error_code() << " msg:" << e.what();
            string errMsg = ss.str();
            LOG_INFO("QueryLoanBorrow AccountId: %d   BinanceLoan Error: '%s' ", accountInfo.accountId, errMsg.c_str());
            vErrorMsg.emplace_back(errMsg);
        }
       */ 
        catch(exception& e) {
            query = false;
            string errMsg = string("BinanceLoan QueryLoanBorrow ") + string(e.what());
            LOG_INFO("QueryLoanBorrow AccountId: %d   BinanceLoan Error: '%s' ", accountInfo.accountId, errMsg.c_str());
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
