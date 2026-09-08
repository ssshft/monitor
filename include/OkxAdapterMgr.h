#pragma once

#include "MonitorConfig.h"
#include "OkxAdapterItem.h"
#include <unordered_map>
#include <set>



class OkxAdapterMgr {
public:
    static OkxAdapterMgr& GetInstance();
	~OkxAdapterMgr();
    void Init(sm::SecurityManager* s);
    void UpdateAccountInfo();   
    std::unordered_map<std::string, md::InstrumentInfo> GetInstrumentList();
    OkxAdapterItem* GetAdapterItem(int accountId);
    
private:
    OkxAdapterMgr();
    std::unordered_map<int, OkxAdapterItem*> mOkxAdapterItem;
};