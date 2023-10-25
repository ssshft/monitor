#pragma once

#include "MonitorConfig.h"
#include "OkxAdapterItem.h"
#include <unordered_map>
#include <set>

using namespace std;


class OkxAdapterMgr {
public:
    static OkxAdapterMgr& GetInstance();
	~OkxAdapterMgr();
    void UpdateAccountInfo();   
    set<string> GetInstrumentList();
    OkxAdapterItem* GetAdapterItem(int accountId);
    
private:
    OkxAdapterMgr();
    unordered_map<int, OkxAdapterItem*> mOkxAdapterItem;
};