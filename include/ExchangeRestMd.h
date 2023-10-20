#pragma once

#include <cpprest/ws_client.h>
#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include "Utility.h"

using namespace web;
using namespace web::websockets::client;
using namespace web::http;
using namespace web::http::client;

class ExchangeRestMd {
public:
    static ExchangeRestMd& GetInstance();
	~ExchangeRestMd();

    Depth GetDepth(string instId, string instType, string exchangeType);
    Depth GetBinanceSpotDepth(string originInstId);
    Depth GetBinanceUFutureDepth(string originInstId);
    Depth GetBinanceCFutureDepth(string originInstId);
    Depth GetGateioSpotDepth(string originInstId);
    Depth GetGateioSwapDepth(string originInstId);
    Depth GetGateioDeliveryDepth(string originInstId);
    Depth GetByBitDepth(string originInstId, string instType);
    Depth GetOkxDepth(string originInstId, string instType);
private:
    ExchangeRestMd();

    web::uri binanceSpotDepthUrl;
    web::uri binanceUFutureDepthUrl;
    web::uri binanceCFutureDepthUrl;

    web::uri gateioSpotDepthUrl;
    web::uri gateioSwapDepthUrl;
    web::uri gateioDeliveryDepthUrl;

    web::uri bybitDepthUrl;

    web::uri okxDepthUrl;
};