#pragma once

#include <cpprest/ws_client.h>
#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include "Utility.h"
#include "binance/BinanceObject.h"
#include "IGException.h"

using namespace web;
using namespace web::websockets::client;
using namespace web::http;
using namespace web::http::client;


class BinanceSpot
{
public:
	BinanceSpot(AccountInfo& info);
	virtual ~BinanceSpot();

    bool QueryAccount(vector<binance::SpotAsset>& vSpotAsset, vector<string>& vErrorMsg);
    vector<binance::SpotTrade> QueryTrades(string symbol, int days);
	vector<binance::SpotOpenOrder> QueryOpenOrder();

private:
	web::uri accountUrl;
    web::uri tradesUrl;
	web::uri openOrderUrl;
	AccountInfo accountInfo;
};
