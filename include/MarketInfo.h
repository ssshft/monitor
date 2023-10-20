#pragma once

#include "Utility.h"
#include "DataBase.h"

class MarketInfo {
public:
    ~MarketInfo();
    static MarketInfo& GetInstance();
    void UpdateSymbolMarketInfo();
    unordered_map<string, SymbolMarketInfo>& GetSymbolMarketInfo();

private:
    MarketInfo();
    DataBase db;
    unordered_map<string, SymbolMarketInfo> mSymbolMarketInfo;
    bool updateFlag;
};