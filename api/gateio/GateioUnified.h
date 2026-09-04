#pragma once

#include "Utility.h"
#include "gateio/GateioObject.h"


class GateioUnified
{
public:
	GateioUnified(AccountInfo& info);
	virtual ~GateioUnified();
    bool QueryAccount(gateio::UnifyTotalAccount& unifyTotalAccount, std::vector<gateio::SpotAsset>& vSpotAsset, std::vector<std::string>& vErrorMsg);

private:
	std::string accountUrl;
	AccountInfo accountInfo;
};