#pragma once

#include "Utility.h"
#include "binance/BinanceObject.h"
#include "IGException.h"


class BinanceCFuture
{
public:
	BinanceCFuture(AccountInfo& info);
	virtual ~BinanceCFuture();

    bool QueryAccount(std::vector<binance::CFutureAsset>& vCFutureAsset, std::vector<binance::CFuturePosition>& vCFuturePosition, std::vector<std::string>& vErrorMsg);
	bool QueryPositionRisk(std::vector<binance::PositionRisk>& vPositionRisk, std::vector<std::string>& vErrorMsg);
	bool QueryOpenOrder(std::vector<binance::FutureOpenOrder>& vOpenOrder, std::vector<std::string>& vErrorMsg);
private:
	std::string baseUrl;
	std::string accountUrl;
	std::string positionRiskUrl;
	std::string openOrderUrl;
	AccountInfo accountInfo;
};
