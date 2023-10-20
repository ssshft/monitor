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


class BinanceMargin
{
public:
	BinanceMargin(AccountInfo& info);
	virtual ~BinanceMargin();

    bool QueryAccount(vector<binance::MarginAsset>& vMarginAsset, vector<string>& vErrorMsg);
	binance::TotalMarginAsset GetTotalAsset();

private:
    web::uri accountUrl;
	AccountInfo accountInfo;
	binance::TotalMarginAsset totalMarginAsset;
};
