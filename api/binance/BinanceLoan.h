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


class BinanceLoan
{
public:
	BinanceLoan(AccountInfo& info);
	virtual ~BinanceLoan();

    bool QueryLoanBorrow(vector<binance::LoanBorrow>& vLoanBorrow, vector<string>& vErrorMsg);

private:
	web::uri loadBorrowUrl;
	AccountInfo accountInfo;
};
