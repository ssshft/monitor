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


class GateioDelivery
{
public:
	GateioDelivery(AccountInfo& info);
	virtual ~GateioDelivery();
    bool QueryAccount(vector<gateio::FutureAsset>& vFutureAsset, vector<string>& vErrorMsg);
    bool QueryPosition(vector<gateio::FuturePosition>& vFuturePosition, vector<string>& vErrorMsg);
    bool QueryOpenOrder(vector<gateio::FutureOrder>& vFutureOrder, vector<string>& vErrorMsg);
    bool QuerySettleAccount(string settle, vector<gateio::FutureAsset>& vFutureAsset, vector<string>& vErrorMsg);
    bool QuerySettlePosition(string settle, vector<gateio::FuturePosition>& vFuturePosition, vector<string>& vErrorMsg);
    bool QuerySettleOpenOrder(string settle, vector<gateio::FutureOrder>& vFutureOrder, vector<string>& vErrorMsg);

private:
	string accountUrl;
    string positionUrl;
    string orderUrl;
	AccountInfo accountInfo;
};