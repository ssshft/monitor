#pragma once

#include "MonitorConfig.h"
#include "CoinbaseAdapterItem.h"
#include <unordered_map>
#include <set>

using namespace std;


class CoinbaseAdapterMgr {
public:
    static CoinbaseAdapterMgr& GetInstance();
	~CoinbaseAdapterMgr();
    void UpdateAccountInfo();   
    set<string> GetInstrumentList();
    CoinbaseAdapterItem* GetAdapterItem(int accountId);
    
private:
    CoinbaseAdapterMgr();
    unordered_map<int, CoinbaseAdapterItem*> mCoinbaseAdapterItem;
};