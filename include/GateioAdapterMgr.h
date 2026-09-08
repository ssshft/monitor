#pragma once

#include "MonitorConfig.h"
#include "GateioAdapterItem.h"
#include <unordered_map>
#include <set>



class GateioAdapterMgr {
public:
    static GateioAdapterMgr& GetInstance();
	~GateioAdapterMgr();
    void Init(sm::SecurityManager* s);
    void UpdateAccountInfo();   
    std::unordered_map<std::string, md::InstrumentInfo>  GetInstrumentList();
    GateioAdapterItem* GetAdapterItem(int accountId);
    
private:
    GateioAdapterMgr();
    std::unordered_map<int, GateioAdapterItem*> mGateioAdapterItem;
};