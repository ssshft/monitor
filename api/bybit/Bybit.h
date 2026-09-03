#pragma once

#include "Utility.h"
#include "bybit/BybitObject.h"


class Bybit
{
public:
	Bybit(AccountInfo& info);
	virtual ~Bybit();
    bool QueryAccount(bybit::TotalAccountInfo& totalAccountInfo, std::vector<bybit::Asset>& vAsset, std::vector<std::string>& vErrorMsg);
    bool QueryPosition(std::vector<bybit::Position>& vPosition, std::vector<std::string>& vErrorMsg);
    bool QueryOpenOrder(std::vector<bybit::Order>& vOpenOrder, std::vector<std::string>& vErrorMsg);

private:
	std::string accountUrl;
    std::string positionUrl;
    std::string orderUrl;
	AccountInfo accountInfo;
};