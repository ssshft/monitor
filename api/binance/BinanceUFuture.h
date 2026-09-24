#pragma once

#include "Utility.h"
#include "binance/BinanceObject.h"
#include "IGException.h"


class BinanceUFuture
{
public:
	BinanceUFuture(AccountInfo& info);
	virtual ~BinanceUFuture();

    bool QueryAccount(std::vector<binance::UFutureAsset>& vUFutureAsset, std::vector<std::string>& vErrorMsg);
	bool QueryPositionRisk(vector<binance::UFuturePosition>& vUFuturePosition, std::vector<std::string>& vErrorMsg);
	bool QueryOpenOrder(std::vector<binance::FutureOpenOrder>& vOpenOrder, std::vector<std::string>& vErrorMsg);

private:
	std::string baseUrl;
	std::string accountUrl;
    std::string tradesUrl;
	std::string positionRiskUrl;
	std::string openOrderUrl;
	AccountInfo accountInfo;
};
