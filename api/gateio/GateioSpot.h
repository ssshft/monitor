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


class GateioSpot
{
public:
	GateioSpot(AccountInfo& info);
	virtual ~GateioSpot();
    bool QueryAccount(vector<gateio::SpotAsset>& vSpotAsset, vector<string>& vErrorMsg);

private:
	string accountUrl;
	AccountInfo accountInfo;
};