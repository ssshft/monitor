#include "MarketInfo.h"
#include "MonitorConfig.h"

MarketInfo::MarketInfo() {
    updateFlag = false;
}

MarketInfo::~MarketInfo() {
}

MarketInfo& MarketInfo::GetInstance() {
	static MarketInfo marketInfo;
	return marketInfo;
}

void MarketInfo::UpdateSymbolMarketInfo() {
    mSymbolMarketInfo.clear();

	string addr = MonitorConfig::GetInstance().GetMarketMysqlAddr();
    int port = MonitorConfig::GetInstance().GetMarketMysqlPort();
    string user = MonitorConfig::GetInstance().GetMarketMysqlUser();
    string password = MonitorConfig::GetInstance().GetMarketMysqlPassword();
    string dbVolume = MonitorConfig::GetInstance().GetMarketMysqlDbVolumeName();
    string dbOpenInterest = MonitorConfig::GetInstance().GetMarketMysqlDbOpenInterestName();

	unordered_map<string, SymbolMarketInfo> mMarketInfoVolume = db.QueryMarketInfoVolume(addr, user, password, dbVolume, port);
    for (auto iter = mMarketInfoVolume.begin(); iter != mMarketInfoVolume.end(); ++iter) {
        auto it = mSymbolMarketInfo.find(iter->first);
        if (it != mSymbolMarketInfo.end()) {
            it->second.volume = iter->second.volume;
        } else {
            mSymbolMarketInfo[iter->first] = iter->second;
        }
    }
    unordered_map<string, SymbolMarketInfo> mMarketInfoOpenInterest = db.QueryMarketInfoOpenInterest(addr, user, password, dbOpenInterest, port);
    for (auto iter = mMarketInfoOpenInterest.begin(); iter != mMarketInfoOpenInterest.end(); ++iter) {
        auto it = mSymbolMarketInfo.find(iter->first);
        if (it != mSymbolMarketInfo.end()) {
            it->second.openInterest = iter->second.openInterest;
        } else {
            mSymbolMarketInfo[iter->first] = iter->second;
        }
    }
    
    // log
    for (auto iter = mSymbolMarketInfo.begin(); iter != mSymbolMarketInfo.end(); ++iter) {
        LOG_INFO("key:%s info --- symbol:%s  volume:%f  openinterest:%f", iter->first.c_str(), iter->second.symbol.c_str(), iter->second.volume, iter->second.openInterest);
    }
}

unordered_map<string, SymbolMarketInfo>& MarketInfo::GetSymbolMarketInfo() {
    tm currentTime = GetCurrentTimeTm();
    if ((currentTime.tm_hour == 0 && (currentTime.tm_min == 0 || currentTime.tm_min == 1)) || !updateFlag) {
        LOG_INFO("UpdateSymbolMarketInfo");
        UpdateSymbolMarketInfo();
        updateFlag = true;
    }
    
    return mSymbolMarketInfo;
}