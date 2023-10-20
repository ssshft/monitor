#pragma once

#include "MonitorConfig.h"
#include "BinanceSystemItem.h"
#include <cpprest/json.h>
#include <unordered_map>
#include <set>

using namespace std;


class BinanceSystemMgr {
public:
    static BinanceSystemMgr& GetInstance();
	~BinanceSystemMgr();
    void OnSubMessage(const web::json::value& content);
    set<string> GetInstrumentList();
    BinanceSystemItem* GetSystemItem(int accountId);
    
private:
    BinanceSystemMgr();
    unordered_map<int, BinanceSystemItem*> mBinanceSystemItem;
};