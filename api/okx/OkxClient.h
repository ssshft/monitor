#pragma once

#include "Utility.h"
#include "okx/OkxObject.h"
#include "IGException.h"


class OkxClient
{
public:
	OkxClient(AccountInfo& info);
	virtual ~OkxClient();

    bool QueryAccount(std::vector<okx::OkxAsset>& vAsset, std::vector<std::string>& vErrorMsg);
	bool QueryPosition(std::vector<okx::OkxPosition>& vPosition, std::vector<std::string>& vErrorMsg);
	bool QueryOpenOrder(std::vector<okx::OkxOrder>& vOpenOrder, std::vector<std::string>& vErrorMsg);

private:
	std::string baseUrl;
	std::string accountUrl;
    std::string positionUrl;
	std::string openOrderUrl;
	AccountInfo accountInfo;
};
