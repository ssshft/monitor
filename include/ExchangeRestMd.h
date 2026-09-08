#pragma once
#include "Utility.h"


class ExchangeRestMd {
public:
    static ExchangeRestMd& GetInstance();
	~ExchangeRestMd();

    Depth GetDepth(ExchangeType exchType, InstType instType, std::string originInstId);
    Depth GetBinanceSpotDepth(string originInstId);
    Depth GetBinanceUFutureDepth(string originInstId);
    Depth GetBinanceCFutureDepth(string originInstId);
    Depth GetGateioSpotDepth(string originInstId);
    Depth GetGateioSwapDepth(string originInstId);
    Depth GetGateioDeliveryDepth(string originInstId);
    Depth GetByBitDepth(InstType instType, string originInstId);
    Depth GetOkxDepth(InstType instType, string originInstId);
private:
    ExchangeRestMd();

    std::string binanceSpotDepthUrl;
    std::string binanceUFutureDepthUrl;
    std::string binanceCFutureDepthUrl;

    std::string gateioSpotDepthUrl;
    std::string gateioSwapDepthUrl;
    std::string gateioDeliveryDepthUrl;

    std::string bybitDepthUrl;

    std::string okxDepthUrl;
};