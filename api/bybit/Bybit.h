#pragma once

#include <cpprest/ws_client.h>
#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include "Utility.h"
#include "bybit/BybitObject.h"

using namespace web;
using namespace web::websockets::client;
using namespace web::http;
using namespace web::http::client;


class Bybit
{
public:
	Bybit(AccountInfo& info);
	virtual ~Bybit();
    bool QueryAccount(bybit::TotalAccountInfo& totalAccountInfo, vector<bybit::Asset>& vAsset, vector<string>& vErrorMsg);
    bool QueryPosition(vector<bybit::Position>& vPosition, vector<string>& vErrorMsg);
    bool QueryOpenOrder(vector<bybit::Order>& vOpenOrder, vector<string>& vErrorMsg);

private:
	string accountUrl;
    string positionUrl;
    string orderUrl;
	AccountInfo accountInfo;
};