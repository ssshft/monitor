#pragma once

#include "MonitorConfig.h"
#include "GateioAdapterItem.h"
#include <unordered_map>
#include <set>

using namespace std;


class GateioAdapterMgr {
public:
    static GateioAdapterMgr& GetInstance();
	~GateioAdapterMgr();
    void UpdateAccountInfo();   
    set<string> GetInstrumentList();
    GateioAdapterItem* GetAdapterItem(int accountId);
    
private:
    GateioAdapterMgr();
    unordered_map<int, GateioAdapterItem*> mGateioAdapterItem;
};