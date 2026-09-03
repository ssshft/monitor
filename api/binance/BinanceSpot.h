#pragma once
#include "Utility.h"
#include "binance/BinanceObject.h"
#include "IGException.h"

class BinanceSpot
{
public:
	BinanceSpot(AccountInfo& info);
	virtual ~BinanceSpot();

    bool QueryAccount(std::vector<binance::SpotAsset>& vSpotAsset, std::vector<std::string>& vErrorMsg);
	bool QueryOpenOrder(std::vector<binance::SpotOpenOrder> vSpotOpenOrder, std::vector<std::string>& vErrorMsg);

private:
	std::string accountUrl;
	std::string openOrderUrl;
	AccountInfo accountInfo;
};
