#pragma once


#include "Utility.h"
#include "binance/BinanceObject.h"


class BinanceStaking
{
public:
	BinanceStaking(AccountInfo& info);
	virtual ~BinanceStaking();

    bool QueryPosition(std::vector<binance::StakingPosition> vStakingPosition, std::vector<std::string>& vErrorMsg);

private:
	std::string positionUrl;
	AccountInfo accountInfo;
};
