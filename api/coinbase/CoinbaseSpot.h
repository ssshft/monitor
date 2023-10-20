#pragma once

#include <cpprest/ws_client.h>
#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include "Utility.h"
#include "CoinbaseObject.h"
#include "IGException.h"

using namespace web;
using namespace web::websockets::client;
using namespace web::http;
using namespace web::http::client;


class CoinbaseSpot
{
public:
	CoinbaseSpot(AccountInfo& info);
	virtual ~CoinbaseSpot();
    void QueryPortfolio();
    bool QueryAccount(vector<coinbase::SpotAsset>& vSpotAsset, vector<string>& vErrorMsg);

private:
    string portfolioUrl;
	string accountUrl;
    string portfolioId;
    string entityId;
    string organizationId;
	AccountInfo accountInfo;
};