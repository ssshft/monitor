#pragma once

#include "MonitorConfig.h"
#include "BinanceAdapterItem.h"
#include <unordered_map>
#include <set>

using namespace std;

class BinanceAdapterMgr {
public:
    static BinanceAdapterMgr& GetInstance();
	~BinanceAdapterMgr();
    void UpdateAccountInfo();   
    set<string> GetInstrumentList();
    BinanceAdapterItem* GetAdapterItem(int accountId);
    
private:
    BinanceAdapterMgr();
    unordered_map<int, BinanceAdapterItem*> mBinanceAdapterItem;
};