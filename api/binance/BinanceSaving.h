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


class BinanceSaving
{
public:
	BinanceSaving(AccountInfo& info);
	virtual ~BinanceSaving();

    bool QueryAccount(vector<binance::SavingAsset>& vSavingAsset, vector<string>& vErrorMsg);
    vector<binance::SavingInterest> QueryInterest();
    vector<binance::SavingPurchase> QueryPurchase();
    vector<binance::SavingRedemption> QueryRedemption();

private:
	web::uri accountUrl;
    web::uri interestUrl;
    web::uri purchaseUrl;
    web::uri redemptionUrl;
	AccountInfo accountInfo;
};
