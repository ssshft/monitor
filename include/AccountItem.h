#pragma once

#include <string>
#include <map>
#include <queue>
#include "Utility.h"
#include "securitymanager.h"


class AccountItem {
public:
	AccountItem(int id, std::string n, std::string ty, std::string ex, int he, sm::SecurityManager* s);
	~AccountItem();
    void UpdateByAdapter();
    void UpdateByBinanceAdapter();
    void UpdateByGateioAdapter();
    void UpdateByBybitAdapter();
    void UpdateByOkxAdapter();
    void ClearZero();
    void Clear();
    void CalculateTotalAsset();
    void UpdateTotalAsset(igmonitor::Asset& asset);
    double GetUnderwayOrderValue(string asset, double frozenAmount);
    void CalculateExposure();
    void UpdateExposure(const md::InstrumentInfo& info, const igmonitor::Position& position);
    void UpdateExposure(igmonitor::Asset& asset);
    //void UpdatePositionFundingRate(igmonitor::Position& position);
    void CalculateRiskInfo();
    std::string GetAccountType();
    int GetAccountId();
    std::string GetExchangeStr();
    igmonitor::RiskInfo& GetRiskInfo();
    std::unordered_map<std::string, igmonitor::Exposure>& GetExposure();
    std::unordered_map<std::string, igmonitor::Asset>& GetSpotAsset();
    std::unordered_map<std::string, igmonitor::Asset>& GetUFutureAsset();
    std::unordered_map<std::string, igmonitor::Asset>& GetCFutureAsset();
    std::unordered_map<std::string, igmonitor::Asset>& GetTotalAsset();
    std::unordered_map<std::string, igmonitor::Position>& GetUFuturePosition();
    std::unordered_map<std::string, igmonitor::Position>& GetCFuturePosition();
    std::unordered_map<std::string, igmonitor::PositionFundingRate>& GetPositionFundingRate();
    std::set<std::string> GetInstrumentList();
    rapidjson::Document GetDetail();
    rapidjson::Document GetPreview();
    std::vector<MsgCard> GetAlarmMsg();
    bool GetAdapterQueryStatus();
    std::vector<MsgCard> GetFundingRateAlarmMsg();
    MsgCard GetLiquidationPriceAlarmMsg(igmonitor::Position& position);
    std::vector<MsgCard> GetPositionLiquidationPriceAlarmMsg();
    std::vector<MsgCard> GetOrderAlarmMsg();
    
private:
    int accountId;
    std::string name;
    std::string type;
    std::string exchangeStr;
    int hedge;
    int unified;
    bool subMsgFlag;

    std::unordered_map<std::string, igmonitor::Asset> mSpotAsset;
    std::unordered_map<std::string, igmonitor::Asset> mUFutureAsset;
    std::unordered_map<std::string, igmonitor::Asset> mCFutureAsset;
    std::unordered_map<std::string, igmonitor::Asset> mPerpetualAsset;
    std::unordered_map<std::string, igmonitor::Position> mUFuturePosition;
    std::unordered_map<std::string, igmonitor::Position> mCFuturePosition;
    std::unordered_map<std::string, igmonitor::Position> mPerpetualPosition;
    std::unordered_map<std::string, igmonitor::Asset> mTotalAsset;
    std::unordered_map<std::string, igmonitor::Exposure> mExposure;
    std::unordered_map<std::string, igmonitor::OpenOrder> mSpotOpenOrder;
    std::unordered_map<std::string, igmonitor::OpenOrder> mUFutureOpenOrder;
    std::unordered_map<std::string, igmonitor::OpenOrder> mCFutureOpenOrder;
    std::unordered_map<std::string, igmonitor::OpenOrder> mPerpetualOpenOrder;

    double totalMarginBalance;
    std::unordered_map<std::string, igmonitor::PositionFundingRate> mPositionFundingRate;
    igmonitor::RiskInfo riskInfo;
    igmonitor::TotalMarAsset totalMarAsset;

    double initialMarginRate;
    double unifyMaintenanceMarginRate;

    std::string baseAsset;
    double MINDOUBLE;

    bool adapterQuery;
    std::vector<std::string> adapterQueryErrMsg;

    int triggerInterval;
    std::unordered_map<int64_t, double> mLeverageAlarm;
    std::unordered_map<int64_t, double> mRiskExposureAlarm;
    std::unordered_map<int64_t, double> mUnderwayOrderValueAlarm;
    AlarmInfo alarmInfo;

    bool maxLeverageUD;
    bool maxLeverageUS;
    std::string maxLeverageAssetD;
    std::string maxLeverageAssetS;
    std::string maxLeverageTabD;
    std::string maxLeverageTabS;
    std::string maxRiskExposureTab;
    std::string maxRiskExposureAssetD;
    std::string maxRiskExposureAssetS;
    
    double totalExposure;

    std::string orderAlarmMsg;

    sm::SecurityManager* smc;
};
