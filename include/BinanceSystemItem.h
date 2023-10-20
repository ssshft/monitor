#pragma once

#include <cpprest/json.h>
#include <string>
#include <unordered_map>
#include <set>
#include "Utility.h"

using namespace std;

class BinanceSystemItem {
public:
    BinanceSystemItem(int id, string n);
    ~BinanceSystemItem();
    void OnSubMessage(const web::json::value& content);
    void UpdateData(const web::json::value& content);
    vector<igsystem::SysAsset>& GetSpotAsset();
    vector<igsystem::SysAsset>& GetUFutureAsset();
    vector<igsystem::SysAsset>& GetCFutureAsset();
    vector<igsystem::SysPosition>& GetUFuturePosition();
    vector<igsystem::SysPosition>& GetCFuturePosition();
    set<string> GetInstrumentList();
    string GetExchangeStr();
    int64_t GetUpdateTime();
private:
    int customerId;
    string name;
    string baseAsset;
    string exchangeStr;
    vector<igsystem::SysAsset> vSpotAsset;
    vector<igsystem::SysAsset> vUFutureAsset;
    vector<igsystem::SysAsset> vCFutureAsset;
    vector<igsystem::SysPosition> vUFuturePosition;
    vector<igsystem::SysPosition> vCFuturePosition;

    int64_t updateTime;
};