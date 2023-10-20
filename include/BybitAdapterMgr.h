#pragma once

#include "MonitorConfig.h"
#include "BybitAdapterItem.h"
#include <unordered_map>
#include <set>

using namespace std;


class BybitAdapterMgr {
public:
    static BybitAdapterMgr& GetInstance();
	~BybitAdapterMgr();
    void UpdateAccountInfo();   
    set<string> GetInstrumentList();
    BybitAdapterItem* GetAdapterItem(int accountId);
    
private:
    BybitAdapterMgr();
    unordered_map<int, BybitAdapterItem*> mBybitAdapterItem;
};