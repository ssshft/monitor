#pragma once

#include "Utility.h"
#include "gateio/GateioObject.h"


class GateioPerpetual
{
public:
	GateioPerpetual(AccountInfo& info);
	virtual ~GateioPerpetual();
    bool QueryAccount(vector<gateio::FutureAsset>& vFutureAsset, vector<string>& vErrorMsg);
    bool QueryPosition(vector<gateio::FuturePosition>& vFuturePosition, vector<string>& vErrorMsg);
    bool QueryOpenOrder(vector<gateio::FutureOrder>& vFutureOrder, vector<string>& vErrorMsg);

private:
	std::string accountUrl;
    std::string positionUrl;
    std::string orderUrl;
	AccountInfo accountInfo;
};