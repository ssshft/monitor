#pragma once

#include <cpprest/ws_client.h>
#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include "Utility.h"
#include "binance/BinanceObject.h"

using namespace web;
using namespace web::websockets::client;
using namespace web::http;
using namespace web::http::client;


class BinanceStaking
{
public:
	BinanceStaking(AccountInfo& info);
	virtual ~BinanceStaking();

    vector<binance::StakingPosition> QueryPosition();

private:
	web::uri positionUrl;
	AccountInfo accountInfo;
};
