#pragma once

#include "MonitorConfig.h"
#include "OkxClient.h"
#include <map>
#include <set>

using namespace std;


class OkxAdapterItem {
public:
    OkxAdapterItem(AccountInfo info);
	~OkxAdapterItem();
    void UpdateAccountInfo();   
    set<string> GetInstrumentList();
    vector<okx::OkxAsset>& GetAsset();
    vector<okx::OkxPosition>& GetPosition();
    vector<okx::OkxOrder>& GetOpenOrder();
    vector<okx::OkxOrder>& GetOrder();
    double GetPositionValue(string asset);
    double GetFloatAmount(string asset);
    void GetLongShortFrozenPosition(string symbol, double longFrozenPos, double shortFrozenPos);
    int64_t GetUpdateTime();
    bool GetQueryStatus();
    vector<string>& GetQueryErrMsg();
    int isUnified();

private:
    AccountInfo accountInfo;
    OkxClient* okxClient;
    vector<okx::OkxAsset> vAsset;
    vector<okx::OkxPosition> vPosition;
    vector<okx::OkxOrder> vOpenOrder;
    vector<okx::OkxOrder> vOrder;
    string baseAsset;
    int64_t updateTime;
    bool query;
    vector<string> vQueryErrMsg;
};
