#pragma once

#include <cpprest/ws_client.h>
#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include "Utility.h"
#include "BinanceObject.h"

using namespace web;
using namespace web::websockets::client;
using namespace web::http;
using namespace web::http::client;


class BinanceBSwap
{
public:
	BinanceBSwap(AccountInfo& info);
	virtual ~BinanceBSwap();

    vector<binance::LiquidityInfo> QueryLiquidity();
    binance::UnclaimedRewards QueryUnclaimedRewards();

private:
	web::uri liquidityUrl;
    web::uri unclaimedRewardsUrl;
	AccountInfo accountInfo;
};
