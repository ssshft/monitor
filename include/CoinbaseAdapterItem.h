#pragma once

#include "MonitorConfig.h"
#include "CoinbaseSpot.h"
#include <map>
#include <set>

using namespace std;


class CoinbaseAdapterItem {
public:
    CoinbaseAdapterItem(AccountInfo info);
	~CoinbaseAdapterItem();
    void UpdateAccountInfo();   
    set<string> GetInstrumentList();
    vector<coinbase::SpotAsset>& GetSpotAsset();
    int64_t GetUpdateTime();
    bool GetQueryStatus();
    vector<string> GetQueryErrMsg();
    int isUnified();

private:
    AccountInfo accountInfo;
    CoinbaseSpot* coinbaseSpot;
    bool spotEnable;
    vector<coinbase::SpotAsset> vSpotAsset;
    string baseAsset;
    int64_t updateTime;

    bool query;
    vector<string> vQueryErrMsg;
};