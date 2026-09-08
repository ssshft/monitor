#pragma once

#include "MonitorConfig.h"
#include "BinanceAdapterItem.h"
#include <unordered_map>
#include <set>


class BinanceAdapterMgr {
public:
    static BinanceAdapterMgr& GetInstance();
	~BinanceAdapterMgr();
    void Init(sm::SecurityManager* s);
    void UpdateAccountInfo();   
    std::unordered_map<std::string, md::InstrumentInfo> GetInstrumentList();
    BinanceAdapterItem* GetAdapterItem(int accountId);
    
private:
    BinanceAdapterMgr();
    std::unordered_map<int, BinanceAdapterItem*> mBinanceAdapterItem;
};