#pragma once

#include <cpprest/ws_client.h>
#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include "Utility.h"
#include "okx/OkxObject.h"
#include "IGException.h"

using namespace web;
using namespace web::websockets::client;
using namespace web::http;
using namespace web::http::client;


class OkxClient
{
public:
	OkxClient(AccountInfo& info);
	virtual ~OkxClient();

    bool QueryAccount(vector<okx::OkxAsset>& vAsset, vector<string>& vErrorMsg);
	bool QueryPosition(vector<okx::OkxPosition>& vPosition, vector<string>& vErrorMsg);
	bool QueryOpenOrder(vector<okx::OkxOrder>& vOpenOrder, vector<string>& vErrorMsg);
    bool QueryOrder(vector<okx::OkxOrder>& vOrder, vector<string>& vErrorMsg);
    bool QueryOrder(vector<okx::OkxOrder>& vOrder, vector<string>& vErrorMsg, string instType);

    std::string base64_encode(unsigned char const * input, size_t len);
    string base64_encode(std::string const & input);
    int HmacEncode(const char * algo,
               const char * key, unsigned int key_length,
               const char * input, unsigned int input_length,
               unsigned char * &output, unsigned int &output_length);
    string getSignature(const string &query,const string &apiSecret);
	string get_signature_rest(const string &timestamp, const string &method,
            const string &requestPath,const string &body);

private:
	web::uri baseUrl;
	web::uri accountUrl;
    web::uri positionUrl;
	web::uri openOrderUrl;
    web::uri historyOrderUrl;
	AccountInfo accountInfo;
};
