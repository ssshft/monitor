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


class BinanceCFuture
{
public:
	BinanceCFuture(AccountInfo& info);
	virtual ~BinanceCFuture();

    bool QueryAccount(vector<binance::CFutureAsset>& vCFutureAsset, vector<binance::CFuturePosition>& vCFuturePosition, vector<string>& vErrorMsg);
    vector<binance::CFutureTrade> QueryTrades(string symbol, int days);
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
