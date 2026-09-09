#pragma once

#include "Utility.h"
#include "gateio/GateioObject.h"




class GateioSpot
{
public:
	GateioSpot(AccountInfo& info);
	virtual ~GateioSpot();
    bool QueryAccount(std::vector<gateio::SpotAsset>& vSpotAsset, std::vector<std::string>& vErrorMsg);
	bool QueryOpenOrder(std::vector<gateio::SpotOrder>& vSpotOpenOrder, std::vector<std::string>& vErrorMsg);

private:
	std::string accountUrl;
	std::string openOrderUrl;
	AccountInfo accountInfo;
};