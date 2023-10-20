#pragma once

#include <cpprest/ws_client.h>
#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include "Utility.h"
#include "BinanceObject.h"
#include "IGException.h"

using namespace web;
using namespace web::websockets::client;
using namespace web::http;
using namespace web::http::client;


class BinanceUFuture
{
public:
	BinanceUFuture(AccountInfo& info);
	virtual ~BinanceUFuture();

    bool QueryAccount(vector<binance::UFutureAsset>& vUFutureAsset, vector<binance::UFuturePosition>& vUFuturePosition, vector<string>& vErrorMsg);
    vector<binance::UFutureTrade> QueryTrades(string symbol, int days);
	bool QueryPositionRisk(vector<binance::PositionRisk>& vPositionRisk, vector<string>& vErrorMsg);
	bool QueryOpenOrder(vector<binance::FutureOpenOrder>& vOpenOrder, vector<string>& vErrorMsg);

private:
	web::uri baseUrl;
	web::uri accountUrl;
    web::uri tradesUrl;
	web::uri positionRiskUrl;
	web::uri openOrderUrl;
	AccountInfo accountInfo;
};
