#pragma once

#include "MonitorConfig.h"
#include "BybitAdapterItem.h"
#include <unordered_map>
#include <set>


class BybitAdapterMgr {
public:
    static BybitAdapterMgr& GetInstance();
	~BybitAdapterMgr();
    void Init(sm::SecurityManager* s);
    void UpdateAccountInfo();   
    std::unordered_map<std::string, md::InstrumentInfo> GetInstrumentList();
    BybitAdapterItem* GetAdapterItem(int accountId);
    
private:
    BybitAdapterMgr();
    std::unordered_map<int, BybitAdapterItem*> mBybitAdapterItem;
};