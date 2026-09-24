#include "ExchangeRestMd.h"
#include "Net.h"


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

Depth ExchangeRestMd::GetDepth(ExchangeType exchType, InstType instType, std::string originInstId) {
    Depth depth;

    if (exchType == BINANCE) {
        if (instType == SPOT) {
            depth = GetBinanceSpotDepth(originInstId);
        }
        else if (instType == USDT_SWAP || instType == USDT_FUTURES) {
            depth = GetBinanceUFutureDepth(originInstId);
        }
        else if (instType == C_SWAP || instType == C_FUTURES) {
            depth = GetBinanceCFutureDepth(originInstId);
        }
    }
    else if (exchType == GATEIO) {
        if (instType == SPOT) {
            depth = GetGateioSpotDepth(originInstId); 
        }
        else if (instType == USDT_SWAP || instType == C_SWAP) {
            depth = GetGateioSwapDepth(originInstId);
        }
        else if (instType == USDT_FUTURES || instType == C_FUTURES) {
            depth = GetGateioDeliveryDepth(originInstId);
        }
    }
    else if (exchType == BYBIT) {
        depth = GetByBitDepth(instType, originInstId);
    }
    else if (exchType == OKX) {
        depth = GetOkxDepth(instType, originInstId);
    }

    depth.instrumentId = originInstId;
    depth.instrumentType = instType;
    depth.exchangeType = exchType;
    depth.marketType = md::DEPTH1;
    return depth;
}

Depth ExchangeRestMd::GetBinanceSpotDepth(string originInstId) {
    Depth depth;

    int status = 0;
    std::string body;
    std::string path = binanceSpotDepthUrl + "?symbol=" + originInstId;
    try {
        if (!Net::Instance().syncGet("BINANCE", crypto::host_of(BINANCE_SPOT_REST), path, {}, {}, body, status)) {
            LOG_ERROR("BINANCE GetBinanceSpotDepth syncGet return false");
            return depth;
        }
        if (status != 200) {
            LOG_INFO("BINANCE GetBinanceSpotDepth status: {}", status);
            return depth;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        double bidPrice = 0.0;
        double bidVolume = 0.0;
        double askPrice = 0.0;
        double askVolume = 0.0;

        if (res.HasMember("bidPrice")) {
            bidPrice = std::stod(res["bidPrice"].GetString());
        }

        if (res.HasMember("bidQty")) {
            bidVolume = std::stod(res["bidQty"].GetString());
        }

        if (res.HasMember("askPrice")) {
            askPrice = std::stod(res["askPrice"].GetString());
        }
        if (res.HasMember("askQty")) {
            askVolume = std::stod(res["askQty"].GetString());
        }

        depth.bidP.push_back(bidPrice);
        depth.bidV.push_back(bidVolume);
        depth.askP.push_back(bidPrice);
        depth.askV.push_back(bidVolume);
        depth.ts = crypto::getCurrentTime();
    }
    catch(exception& e) {
        LOG_INFO("GetBinanceSpotDepth Error: {}", e.what());
    }

    return depth;
}

Depth ExchangeRestMd::GetBinanceUFutureDepth(string originInstId) {
    Depth depth;

    int status = 0;
    std::string body;
    std::string path = binanceUFutureDepthUrl + "?symbol=" + originInstId;
    try {
        if (!Net::Instance().syncGet("BINANCE", crypto::host_of(BINANCE_UFUTURE_REST), path, {}, {}, body, status)) {
            LOG_ERROR("BINANCE GetBinanceUFutureDepth syncGet return false");
            return depth;
        }
        if (status != 200) {
            LOG_INFO("BINANCE GetBinanceUFutureDepth status: {}", status);
            return depth;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        double bidPrice = 0.0;
        double bidVolume = 0.0;
        double askPrice = 0.0;
        double askVolume = 0.0;

        if (res.HasMember("bidPrice")) {
            bidPrice = std::stod(res["bidPrice"].GetString());
        }

        if (res.HasMember("bidQty")) {
            bidVolume = std::stod(res["bidQty"].GetString());
        }

        if (res.HasMember("askPrice")) {
            askPrice = std::stod(res["askPrice"].GetString());
        }
        if (res.HasMember("askQty")) {
            askVolume = std::stod(res["askQty"].GetString());
        }

        depth.bidP.push_back(bidPrice);
        depth.bidV.push_back(bidVolume);
        depth.askP.push_back(bidPrice);
        depth.askV.push_back(bidVolume);
        depth.ts = crypto::getCurrentTime();
    }
    catch(exception& e) {
        LOG_INFO("GetBinanceUFutureDepth Error: {}", e.what());
    }

    return depth;
}

Depth ExchangeRestMd::GetBinanceCFutureDepth(string originInstId) {
    Depth depth;

    int status = 0;
    std::string body;
    std::string path = binanceCFutureDepthUrl + "?symbol=" + originInstId;
    try {
        if (!Net::Instance().syncGet("BINANCE", crypto::host_of(BINANCE_CFUTURE_REST), path, {}, {}, body, status)) {
            LOG_ERROR("BINANCE GetBinanceCFutureDepth syncGet return false");
            return depth;
        }
        if (status != 200) {
            LOG_INFO("BINANCE GetBinanceCFutureDepth status: {}", status);
            return depth;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        double bidPrice = 0.0;
        double bidVolume = 0.0;
        double askPrice = 0.0;
        double askVolume = 0.0;

        if (res.HasMember("bidPrice")) {
            bidPrice = std::stod(res["bidPrice"].GetString());
        }

        if (res.HasMember("bidQty")) {
            bidVolume = std::stod(res["bidQty"].GetString());
        }

        if (res.HasMember("askPrice")) {
            askPrice = std::stod(res["askPrice"].GetString());
        }
        if (res.HasMember("askQty")) {
            askVolume = std::stod(res["askQty"].GetString());
        }

        depth.bidP.push_back(bidPrice);
        depth.bidV.push_back(bidVolume);
        depth.askP.push_back(bidPrice);
        depth.askV.push_back(bidVolume);
        depth.ts = crypto::getCurrentTime();
    }
    catch(exception& e) {
        LOG_INFO("GetBinanceCFutureDepth Error: {}", e.what());
    }

    return depth;
}

Depth ExchangeRestMd::GetGateioSpotDepth(string originInstId) {
    Depth depth;
    int status = 0;
    std::string body;
    std::string path = gateioSpotDepthUrl + "?currency_pair=" + originInstId;
    try {
        if (!Net::Instance().syncGet("GATEIO", crypto::host_of(GATEIO_REST), path, {}, {}, body, status)) {
            LOG_ERROR("GATEIO GetGateioSpotDepth syncGet return false");
            return depth;
        }
        if (status != 200) {
            LOG_ERROR("GATEIO GetGateioSpotDepth status: {}", status);
            return depth;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        double bidPrice = 0.0;
        double bidVolume = 0.0;
        double askPrice = 0.0;
        double askVolume = 0.0;

        if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                if (res[i].HasMember("lowest_bid")) {
                    bidPrice = std::stod(res[i]["lowest_bid"].GetString());
                }

                if (res[i].HasMember("lowest_ask")) {
                    askPrice = std::stod(res[i]["lowest_ask"].GetString());
                }
            }
        }

        depth.bidP.push_back(bidPrice);
        depth.bidV.push_back(bidVolume);
        depth.askP.push_back(bidPrice);
        depth.askV.push_back(bidVolume);
        depth.ts = crypto::getCurrentTime();
    }
    catch(exception& e) {
        LOG_INFO("GetGateioSpotDepth Error: {}", e.what());
    }

    return depth;
}

Depth ExchangeRestMd::GetGateioSwapDepth(string originInstId) {
    Depth depth;

    int status = 0;
    std::string body;
    std::string path = gateioSwapDepthUrl + "?contract=" + originInstId;

    std::cout << "GetGateioSwapDepth: " << path << std::endl;
    try {
        if (!Net::Instance().syncGet("GATEIO", crypto::host_of(GATEIO_REST), path, {}, {}, body, status)) {
            LOG_ERROR("GATEIO GetGateioSwapDepth syncGet return false");
            return depth;
        }
        if (status != 200) {
            LOG_ERROR("GATEIO GetGateioSwapDepth status: {}", status);
            return depth;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        double bidPrice = 0.0;
        double bidVolume = 0.0;
        double askPrice = 0.0;
        double askVolume = 0.0;

        if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                if (res[i].HasMember("last")) {
                    bidPrice = std::stod(res[i]["last"].GetString());
                    askPrice = bidPrice;
                }
            }
        }

        depth.bidP.push_back(bidPrice);
        depth.bidV.push_back(bidVolume);
        depth.askP.push_back(bidPrice);
        depth.askV.push_back(bidVolume);
        depth.ts = crypto::getCurrentTime();
    }
    catch(exception& e) {
        LOG_INFO("GetGateioSwapDepth Error: {}", e.what());
    }

    return depth;
}

Depth ExchangeRestMd::GetGateioDeliveryDepth(string originInstId) {
    Depth depth;

    int status = 0;
    std::string body;
    std::string path = gateioDeliveryDepthUrl + "?contract=" + originInstId;
    try {
        if (!Net::Instance().syncGet("GATEIO", crypto::host_of(GATEIO_REST), path, {}, {}, body, status)) {
            LOG_ERROR("GATEIO GetGateioDeliveryDepth syncGet return false");
            return depth;
        }
        if (status != 200) {
            LOG_ERROR("GATEIO GetGateioDeliveryDepth status: {}", status);
            return depth;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        double bidPrice = 0.0;
        double bidVolume = 0.0;
        double askPrice = 0.0;
        double askVolume = 0.0;

        if (res.IsArray()) {
            for (rapidjson::SizeType i = 0; i < res.Size(); ++i) {
                if (res[i].HasMember("last")) {
                    bidPrice = std::stod(res[i]["last"].GetString());
                    askPrice = bidPrice;
                }
            }
        }

        depth.bidP.push_back(bidPrice);
        depth.bidV.push_back(bidVolume);
        depth.askP.push_back(bidPrice);
        depth.askV.push_back(bidVolume);
        depth.ts = crypto::getCurrentTime();
    }
    catch(exception& e) {
        LOG_INFO("GetGateioDeliveryDepth Error: {}", e.what());
    }

    return depth;
}

Depth ExchangeRestMd::GetByBitDepth(InstType instType, string originInstId) {
    Depth depth;

    int status = 0;
    std::string body;

    string category = "spot";
    if (instType == SPOT) {
        category = "spot";
    } else {
        category = "linear";
    }
    std::string path = bybitDepthUrl + "?symbol=" + originInstId + "&category=" + category;
    try {
        if (!Net::Instance().syncGet("BYBIT", crypto::host_of(BYBIT_REST), path, {}, {}, body, status)) {
            LOG_ERROR("BYBIT GetByBitDepth syncGet return false");
            return depth;
        }
        if (status != 200) {
            LOG_ERROR("BYBIT GetByBitDepth status: {}", status);
            return depth;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        double bidPrice = 0.0;
        double bidVolume = 0.0;
        double askPrice = 0.0;
        double askVolume = 0.0;

        if (res.HasMember("result")) {
            if (res["result"].HasMember("list")) {
                const rapidjson::Value& list = res["result"]["list"];
                for (rapidjson::SizeType i = 0; i < list.Size(); ++i) {
                    if (list[i].HasMember("bid1Price")) {
                        bidPrice = std::stod(list[i]["bid1Price"].GetString());
                    }

                    if (list[i].HasMember("bid1Size")) {
                        bidVolume = std::stod(list[i]["bid1Size"].GetString());
                    }

                    if (list[i].HasMember("ask1Price")) {
                        askPrice = std::stod(list[i]["ask1Price"].GetString());
                    }

                    if (list[i].HasMember("ask1Size")) {
                        askVolume = std::stod(list[i]["ask1Size"].GetString());
                    }
                }
            }
        }

        depth.bidP.push_back(bidPrice);
        depth.bidV.push_back(bidVolume);
        depth.askP.push_back(bidPrice);
        depth.askV.push_back(bidVolume);
        depth.ts = crypto::getCurrentTime();
    }
    catch(exception& e) {
        LOG_INFO("GetByBitDepth Error: {}", e.what());
    }

    return depth;
}

Depth ExchangeRestMd::GetOkxDepth(InstType instType, string originInstId) {
    Depth depth;

    int status = 0;
    std::string body;
    std::string path = okxDepthUrl + "?instId=" + originInstId;
    try {
        if (!Net::Instance().syncGet("OKX", crypto::host_of(OKX_REST), path, {}, {}, body, status)) {
            LOG_ERROR("OKX GetOkxDepth syncGet return false");
            return depth;
        }
        if (status != 200) {
            LOG_ERROR("OKX GetOkxDepth status: {}", status);
            return depth;
        }

        rapidjson::Document d;
        rapidjson::Value &res = d.Parse<rapidjson::kParseNumbersAsStringsFlag>(body.c_str());

        double bidPrice = 0.0;
        double bidVolume = 0.0;
        double askPrice = 0.0;
        double askVolume = 0.0;

        if (res.HasMember("data")) {
            const rapidjson::Value& data = res["data"];
            for (rapidjson::SizeType i = 0; i < data.Size(); ++i) {
                if (data[i].HasMember("bidPx")) {
                    bidPrice = std::stod(data[i]["bidPx"].GetString());
                }

                if (data[i].HasMember("bidSz")) {
                    bidVolume = std::stod(data[i]["bidSz"].GetString());
                }

                if (data[i].HasMember("askPx")) {
                    askPrice = std::stod(data[i]["askPx"].GetString());
                }

                if (data[i].HasMember("askSz")) {
                    askVolume = std::stod(data[i]["askSz"].GetString());
                }
            }
        }

        depth.bidP.push_back(bidPrice);
        depth.bidV.push_back(bidVolume);
        depth.askP.push_back(bidPrice);
        depth.askV.push_back(bidVolume);
        depth.ts = crypto::getCurrentTime();
    }
    catch(exception& e) {
        LOG_INFO("GetOkxDepth Error: {}", e.what());
    }

    return depth;
}