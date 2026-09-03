#pragma once


#include "Utility.h"
#include "binance/BinanceObject.h"
#include "IGException.h"


class BinanceUnify
{
public:
	BinanceUnify(AccountInfo& info);
	virtual ~BinanceUnify();
    bool QueryBalance(std::vector<binance::UnifyAsset>& vUnifyAsset, std::vector<std::string>& vErrorMsg);
    bool QueryAccount(binance::UnifyAccount& unifyAccount, std::vector<std::string>& vErrorMsg);
    bool QueryUmPosition(std::vector<binance::UnifyPosition>& vUnifyPosition, std::vector<std::string>& vErrorMsg);
    bool QueryCmPosition(std::vector<binance::UnifyPosition>& vUnifyPosition, std::vector<std::string>& vErrorMsg);
	bool QueryUmOpenOrder(std::vector<binance::UnifyOpenOrder>& vOpenOrder, std::vector<std::string>& vErrorMsg);
    bool QueryCmOpenOrder(std::vector<binance::UnifyOpenOrder>& vOpenOrder, std::vector<std::string>& vErrorMsg);

private:
	std::string baseUrl;
    std::string balanceUrl;
	std::string accountUrl;
    std::string umPositionUrl;
	std::string cmPositionUrl;
	std::string umOpenOrderUrl;
    std::string cmOpenOrderUrl;
	AccountInfo accountInfo;
};
