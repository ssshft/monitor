#include "ExchangeRestMd.h"
#include "BasicInfoMgr.h"

ExchangeRestMd::ExchangeRestMd() {
    binanceSpotDepthUrl = "/api/v3/ticker/bookTicker";
    binanceUFutureDepthUrl = "/fapi/v1/ticker/bookTicker";
    binanceCFutureDepthUrl = "/dapi/v1/ticker/bookTicker";

    gateioSpotDepthUrl = "/spot/tickers";
    gateioSwapDepthUrl = "/futures/usdt/tickers";
    gateioDeliveryDepthUrl = "/delivery/usdt/tickers";

    bybitDepthUrl = "/v5/market/tickers";

    okxDepthUrl = "/api/v5/market/ticker";
}

ExchangeRestMd::~ExchangeRestMd() {

}

ExchangeRestMd& ExchangeRestMd::GetInstance() {
	static ExchangeRestMd exchangeRestMd;
	return exchangeRestMd;
}

Depth ExchangeRestMd::GetDepth(string instId, string instType, string exchangeType) {
    Depth depth;
    string key = exchangeType + "|" + instId + "|" + instType;
    InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
    string originInstId = info.originInstrumentId;
    if (exchangeType == "BINANCE") {
        if (instType == "SPOT" || instType == "InstType_SPOT") {
            depth = GetBinanceSpotDepth(originInstId);
        } else if (instType == "InstType_USDT_SWAP" || instType == "InstType_USDT_FUTURES" || info.calculateType == 0) {
            depth = GetBinanceUFutureDepth(originInstId);
        } else if (instType == "InstType_C_SWAP" || instType == "InstType_C_FUTURES" || info.calculateType == 1) {
            depth = GetBinanceCFutureDepth(originInstId);
        }
    } else if (exchangeType == "GATEIO") {
        if (instType == "SPOT" || instType == "InstType_SPOT") {
            depth = GetGateioSpotDepth(originInstId); 
        } else if (instType == "SWAP" || instType == "InstType_USDT_SWAP" || instType == "InstType_C_SWAP") {
            depth = GetGateioSwapDepth(originInstId);
        } else if (instType == "FUTURES" || instType == "InstType_USDT_FUTURES" || instType == "InstType_C_FUTURES") {
            depth = GetGateioDeliveryDepth(originInstId);
        }
    } else if (exchangeType == "BYBIT") {
        depth = GetByBitDepth(originInstId, instType);
    } else if (exchangeType == "OKX") {
        depth = GetOkxDepth(originInstId, instType);
    }
    depth.instrumentId = instId;
    depth.instrumentType = instType;
    depth.exchangeType = exchangeType;
    depth.marketType = "DEPTH1";
    return depth;
}

Depth ExchangeRestMd::GetBinanceSpotDepth(string originInstId) {
    Depth depth;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(BINANCE_SPOT_REST, config);
        http_request request(methods::GET);
        uri_builder builder(binanceSpotDepthUrl);
        builder.append_query("symbol", originInstId);    

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

            LOG_INFO("GetBinanceSpotDepth response: '%s' ", response.to_string().c_str());
            // throw IGException(response.to_string().c_str(), code);
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();

            double bidPrice = 0.0;
            double bidVolume = 0.0;
            double askPrice = 0.0;
            double askVolume = 0.0;
            if (content.has_field("bidPrice")) {
                bidPrice = stod(content.at("bidPrice").as_string());
            }
            if (content.has_field("bidQty")) {
                bidVolume = stod(content.at("bidQty").as_string());
            }
            if (content.has_field("askPrice")) {
                askPrice = stod(content.at("askPrice").as_string());
            }
            if (content.has_field("askQty")) {
                askVolume = stod(content.at("askQty").as_string());
            }

            depth.bidP.push_back(bidPrice);
            depth.bidV.push_back(bidVolume);
            depth.askP.push_back(bidPrice);
            depth.askV.push_back(bidVolume);
            depth.ts = GetCurrentTimeUs();
            LOG_INFO("GetBinanceSpotDepth symbol:%s bidPrice:%f bidVolume:%f askPrice:%f askVolume:%f", originInstId.c_str(), bidPrice, bidVolume, askPrice, askVolume);
        })
        .wait();
    }
    catch(exception& e) {
        LOG_INFO("GetBinanceSpotDepth Error: '%s' ", e.what());
    }
    return depth;
}

Depth ExchangeRestMd::GetBinanceUFutureDepth(string originInstId) {
    Depth depth;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(BINANCE_UFUTURE_REST, config);
        http_request request(methods::GET);
        uri_builder builder(binanceUFutureDepthUrl);
        builder.append_query("symbol", originInstId);    

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

            LOG_INFO("GetBinanceUFutureDepth response: '%s' ", response.to_string().c_str());
            // throw IGException(response.to_string().c_str(), code);
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();

            double bidPrice = 0.0;
            double bidVolume = 0.0;
            double askPrice = 0.0;
            double askVolume = 0.0;
            if (content.has_field("bidPrice")) {
                bidPrice = stod(content.at("bidPrice").as_string());
            }
            if (content.has_field("bidQty")) {
                bidVolume = stod(content.at("bidQty").as_string());
            }
            if (content.has_field("askPrice")) {
                askPrice = stod(content.at("askPrice").as_string());
            }
            if (content.has_field("askQty")) {
                askVolume = stod(content.at("askQty").as_string());
            }

            depth.bidP.push_back(bidPrice);
            depth.bidV.push_back(bidVolume);
            depth.askP.push_back(bidPrice);
            depth.askV.push_back(bidVolume);
            depth.ts = GetCurrentTimeUs();
            LOG_INFO("GetBinanceUFutureDepth symbol:%s bidPrice:%f bidVolume:%f askPrice:%f askVolume:%f", originInstId.c_str(), bidPrice, bidVolume, askPrice, askVolume);
        })
        .wait();
    }
    catch(exception& e) {
        LOG_INFO("GetBinanceUFutureDepth Error: '%s' ", e.what());
    }
    return depth;
}

Depth ExchangeRestMd::GetBinanceCFutureDepth(string originInstId) {
    Depth depth;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(BINANCE_CFUTURE_REST, config);
        http_request request(methods::GET);
        uri_builder builder(binanceCFutureDepthUrl);
        builder.append_query("symbol", originInstId);    

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

            LOG_INFO("GetBinanceCFutureDepth response: '%s' ", response.to_string().c_str());
            // throw IGException(response.to_string().c_str(), code);
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();

            double bidPrice = 0.0;
            double bidVolume = 0.0;
            double askPrice = 0.0;
            double askVolume = 0.0;
            if (content.has_field("bidPrice")) {
                bidPrice = stod(content.at("bidPrice").as_string());
            }
            if (content.has_field("bidQty")) {
                bidVolume = stod(content.at("bidQty").as_string());
            }
            if (content.has_field("askPrice")) {
                askPrice = stod(content.at("askPrice").as_string());
            }
            if (content.has_field("askQty")) {
                askVolume = stod(content.at("askQty").as_string());
            }

            depth.bidP.push_back(bidPrice);
            depth.bidV.push_back(bidVolume);
            depth.askP.push_back(bidPrice);
            depth.askV.push_back(bidVolume);
            depth.ts = GetCurrentTimeUs();
            LOG_INFO("GetBinanceCFutureDepth symbol:%s bidPrice:%f bidVolume:%f askPrice:%f askVolume:%f", originInstId.c_str(), bidPrice, bidVolume, askPrice, askVolume);
        })
        .wait();
    }
    catch(exception& e) {
        LOG_INFO("GetBinanceCFutureDepth Error: '%s' ", e.what());
    }
    return depth;
}

Depth ExchangeRestMd::GetGateioSpotDepth(string originInstId) {
    Depth depth;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(GATEIO_REST, config);
        http_request request(methods::GET);
        uri_builder builder(gateioSpotDepthUrl);
        builder.append_query("currency_pair", originInstId);    

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

            LOG_INFO("GetGateioSpotDepth response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();
            auto contractArray = content.as_array();
            double bidPrice = 0.0;
            double bidVolume = 0.0;
            double askPrice = 0.0;
            double askVolume = 0.0;
            for (auto& contract : contractArray) {
                if (contract.has_field("lowest_ask")) {
                    bidPrice = stod(contract.at("lowest_ask").as_string());
                }
                if (contract.has_field("lowest_ask")) {
                    askPrice = stod(contract.at("lowest_ask").as_string());
                }
            }

            depth.bidP.push_back(bidPrice);
            depth.bidV.push_back(bidVolume);
            depth.askP.push_back(bidPrice);
            depth.askV.push_back(bidVolume);
            depth.ts = GetCurrentTimeUs();
            LOG_INFO("GetGateioSpotDepth symbol:%s bidPrice:%f bidVolume:%f askPrice:%f askVolume:%f", originInstId.c_str(), bidPrice, bidVolume, askPrice, askVolume);
        })
        .wait();
    }
    catch(exception& e) {
        LOG_INFO("GetGateioSpotDepth Error: '%s' ", e.what());
    }
    return depth;
}

Depth ExchangeRestMd::GetGateioSwapDepth(string originInstId) {
    Depth depth;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(GATEIO_REST, config);
        http_request request(methods::GET);
        uri_builder builder(gateioSwapDepthUrl);
        builder.append_query("contract", originInstId);    

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

            LOG_INFO("GetGateioSwapDepth response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();
            auto contractArray = content.as_array();
            double bidPrice = 0.0;
            double bidVolume = 0.0;
            double askPrice = 0.0;
            double askVolume = 0.0;
            for (auto& contract : contractArray) {
                if (contract.has_field("last")) {
                    bidPrice = stod(contract.at("last").as_string());
                    askPrice = bidPrice;
                }
            }

            depth.bidP.push_back(bidPrice);
            depth.bidV.push_back(bidVolume);
            depth.askP.push_back(bidPrice);
            depth.askV.push_back(bidVolume);
            depth.ts = GetCurrentTimeUs();
            LOG_INFO("GetGateioSwapDepth symbol:%s bidPrice:%f bidVolume:%f askPrice:%f askVolume:%f", originInstId.c_str(), bidPrice, bidVolume, askPrice, askVolume);
        })
        .wait();
    }
    catch(exception& e) {
        LOG_INFO("GetGateioSwapDepth Error: '%s' ", e.what());
    }
    return depth;
}

Depth ExchangeRestMd::GetGateioDeliveryDepth(string originInstId) {
    Depth depth;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(GATEIO_REST, config);
        http_request request(methods::GET);
        uri_builder builder(gateioDeliveryDepthUrl);
        builder.append_query("contract", originInstId);    

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

            LOG_INFO("GetGateioDeliveryDepth response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            json::value const& content = previousTask.get();
            auto contractArray = content.as_array();
            double bidPrice = 0.0;
            double bidVolume = 0.0;
            double askPrice = 0.0;
            double askVolume = 0.0;
            for (auto& contract : contractArray) {
                if (contract.has_field("last")) {
                    bidPrice = stod(contract.at("last").as_string());
                    askPrice = bidPrice;
                }
            }

            depth.bidP.push_back(bidPrice);
            depth.bidV.push_back(bidVolume);
            depth.askP.push_back(bidPrice);
            depth.askV.push_back(bidVolume);
            depth.ts = GetCurrentTimeUs();
            LOG_INFO("GetGateioDeliveryDepth symbol:%s bidPrice:%f bidVolume:%f askPrice:%f askVolume:%f", originInstId.c_str(), bidPrice, bidVolume, askPrice, askVolume);
        })
        .wait();
    }
    catch(exception& e) {
        LOG_INFO("GetGateioDeliveryDepth Error: '%s' ", e.what());
    }
    return depth;
}

Depth ExchangeRestMd::GetByBitDepth(string originInstId, string instType) {
    Depth depth;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(BYBIT_REST, config);
        http_request request(methods::GET);
        uri_builder builder(bybitDepthUrl);
        builder.append_query("symbol", originInstId);

        string category = "spot";
        if (instType == "SPOT" || instType == "InstType_SPOT") {
            category = "spot";
        } else {
            category = "linear";
        }
        builder.append_query("category", category);

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

            LOG_INFO("GetByBitDepth response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            double bidPrice = 0.0;
            double bidVolume = 0.0;
            double askPrice = 0.0;
            double askVolume = 0.0;

            json::value const& content = previousTask.get();
            if (content.has_field("result")) {
                auto& res = content.at("result");
                if (res.has_field("list")) {
                    auto& li = res.at("list");
                    if (li.is_array()) {
                        auto& liArray = li.as_array();
                        for (auto& arr: liArray) {
                            if (arr.has_field("bid1Price")) {
                                bidPrice = stod(arr.at("bid1Price").as_string());
                            }
                            if (arr.has_field("bid1Size")) {
                                bidVolume = stod(arr.at("bid1Size").as_string());
                            }
                            if (arr.has_field("ask1Price")) {
                                askPrice = stod(arr.at("ask1Price").as_string());
                            }
                            if (arr.has_field("ask1Size")) {
                                askVolume = stod(arr.at("ask1Size").as_string());
                            }
                        }
                    }
                }
            }

            depth.bidP.push_back(bidPrice);
            depth.bidV.push_back(bidVolume);
            depth.askP.push_back(bidPrice);
            depth.askV.push_back(bidVolume);
            depth.ts = GetCurrentTimeUs();
            LOG_INFO("GetByBitDepth symbol:%s bidPrice:%f bidVolume:%f askPrice:%f askVolume:%f", originInstId.c_str(), bidPrice, bidVolume, askPrice, askVolume);
        })
        .wait();
    }
    catch(exception& e) {
        LOG_INFO("GetByBitDepth Error: '%s' ", e.what());
    }
    return depth;
}

Depth ExchangeRestMd::GetOkxDepth(string originInstId, string instType) {
    Depth depth;
    try {
        http_client_config config;
        config.set_timeout(utility::seconds(5));
        http_client client(OKX_REST, config);
        http_request request(methods::GET);
        uri_builder builder(okxDepthUrl);
        builder.append_query("instId", originInstId);

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

            LOG_INFO("GetOkxDepth response: '%s' ", response.to_string().c_str());
            throw exception();
            return pplx::task_from_result(json::value());  // return an empty JSON value
        })
        .then([&](pplx::task<json::value> previousTask) {  // get the JSON value from the task and display content from it
            double bidPrice = 0.0;
            double bidVolume = 0.0;
            double askPrice = 0.0;
            double askVolume = 0.0;

            json::value const& content = previousTask.get();
            if (content.has_field("data")) {
                auto& li = content.at("data");
                if (li.is_array()) {
                    auto& liArray = li.as_array();
                    for (auto& arr: liArray) {
                        if (arr.has_field("bidPx")) {
                            bidPrice = stod(arr.at("bidPx").as_string());
                        }
                        if (arr.has_field("bidSz")) {
                            bidVolume = stod(arr.at("bidSz").as_string());
                        }
                        if (arr.has_field("askPx")) {
                            askPrice = stod(arr.at("askPx").as_string());
                        }
                        if (arr.has_field("askSz")) {
                            askVolume = stod(arr.at("askSz").as_string());
                        }
                    }
                }  
            }

            depth.bidP.push_back(bidPrice);
            depth.bidV.push_back(bidVolume);
            depth.askP.push_back(bidPrice);
            depth.askV.push_back(bidVolume);
            depth.ts = GetCurrentTimeUs();
            LOG_INFO("GetOkxDepth symbol:%s bidPrice:%f bidVolume:%f askPrice:%f askVolume:%f", originInstId.c_str(), bidPrice, bidVolume, askPrice, askVolume);
        })
        .wait();
    }
    catch(exception& e) {
        LOG_INFO("GetOkxDepth Error: '%s' ", e.what());
    }
    return depth;
}