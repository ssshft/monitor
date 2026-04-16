#pragma once

#include <cpprest/ws_client.h>
#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include "Utility.h"
#include "gateio/GateioObject.h"

using namespace web;
using namespace web::websockets::client;
using namespace web::http;
using namespace web::http::client;


class GateioCrossMargin
{
public:
	GateioCrossMargin(AccountInfo& info);
	virtual ~GateioCrossMargin();
    bool QueryAccount(vector<gateio::CrossMarginAsset>& vCrossMarginAsset, gateio::CrossMarginAccountTotal& crossMarginAccountTotal, vector<string>& vErrorMsg);

private:
	string accountUrl;
	AccountInfo accountInfo;
};