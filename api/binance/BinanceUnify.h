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


class BinanceUnify
{
public:
	BinanceUnify(AccountInfo& info);
	virtual ~BinanceUnify();
    bool QueryBalance(vector<binance::UnifyAsset>& vUnifyAsset, vector<string>& vErrorMsg);
    bool QueryAccount(binance::UnifyAccount& unifyAccount, vector<string>& vErrorMsg);
    bool QueryUmPosition(vector<binance::UnifyPosition>& vUnifyPosition, vector<string>& vErrorMsg);
    bool QueryCmPosition(vector<binance::UnifyPosition>& vUnifyPosition, vector<string>& vErrorMsg);
	bool QueryUmOpenOrder(vector<binance::UnifyOpenOrder>& vOpenOrder, vector<string>& vErrorMsg);
    bool QueryCmOpenOrder(vector<binance::UnifyOpenOrder>& vOpenOrder, vector<string>& vErrorMsg);

private:
	web::uri baseUrl;
    web::uri balanceUrl;
	web::uri accountUrl;
    web::uri umPositionUrl;
	web::uri cmPositionUrl;
	web::uri umOpenOrderUrl;
    web::uri cmOpenOrderUrl;
	AccountInfo accountInfo;
};
