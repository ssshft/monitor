#pragma once

#include <cpprest/json.h>
#include <string>
#include <map>
#include <queue>
#include "Utility.h"

using namespace std;


class BinanceAccountItem {
public:
	BinanceAccountItem(int id, string n, string ty, string ex, int he);
	~BinanceAccountItem();
    void UpdateBySystem();
    void UpdateByAdapter();
    void UpdateByBinanceAdapter();
    void UpdateByCoinbaseAdapter();
    void UpdateByGateioAdapter();
    void UpdateByBybitAdapter();
    void UpdateByOkxAdapter();
    void ClearZero();
    void Clear();
    void CalculateTotalAsset();
    void UpdateTotalAsset(igmonitor::Asset& asset);
    void UpdateTotalAsset(igmonitor::MarAsset& asset);
    void UpdateTotalAsset(igmonitor::SavAsset& asset);
    void UpdateTotalAsset(igmonitor::LoBo& lb);
    double GetUnderwayOrderValue(string asset, double frozenAmount);
    void CalculateExposure();
    void UpdateExposure(string positionType, igmonitor::Position& position);
    void UpdateExposure(igmonitor::Asset& asset);
    void UpdatePositionFundingRate(igmonitor::Position& position);
    void CalculateRiskInfo();
    string GetAccountType();
    int GetAccountId();
    string GetExchangeStr();
    igmonitor::RiskInfo& GetRiskInfo();
    unordered_map<string, igmonitor::Exposure>& GetExposure();
    unordered_map<string, igmonitor::Asset>& GetSpotAsset();
    unordered_map<string, igmonitor::Asset>& GetUFutureAsset();
    unordered_map<string, igmonitor::Asset>& GetCFutureAsset();
    unordered_map<string, igmonitor::Asset>& GetTotalAsset();
    unordered_map<string, igmonitor::Position>& GetUFuturePosition();
    unordered_map<string, igmonitor::Position>& GetCFuturePosition();
    unordered_map<string, igmonitor::PositionFundingRate>& GetPositionFundingRate();
    set<string> GetInstrumentList();
    web::json::value GetDetail();
    web::json::value GetPreview();
    vector<MsgCard> GetAlarmMsg();
    bool GetAdapterQueryStatus();
    vector<MsgCard> GetFundingRateAlarmMsg();
    MsgCard GetLiquidationPriceAlarmMsg(igmonitor::Position& position);
    vector<MsgCard> GetPositionLiquidationPriceAlarmMsg();
    
private:
    int customerId;
    string name;
    string type;
    string exchangeStr;
    int hedge;
    int unified;
    bool subMsgFlag;

    unordered_map<string, igmonitor::Asset> mSpotAsset;
    unordered_map<string, igmonitor::Asset> mUFutureAsset;
    unordered_map<string, igmonitor::Asset> mCFutureAsset;
    unordered_map<string, igmonitor::Asset> mDeliveryAsset;
    unordered_map<string, igmonitor::Asset> mPerpetualAsset;
    unordered_map<string, igmonitor::Position> mUFuturePosition;
    unordered_map<string, igmonitor::Position> mCFuturePosition;
    unordered_map<string, igmonitor::Position> mDeliveryPosition;
    unordered_map<string, igmonitor::Position> mPerpetualPosition;
    unordered_map<string, igmonitor::Asset> mTotalAsset;
    unordered_map<string, igmonitor::Exposure> mExposure;
    unordered_map<string, igmonitor::SavAsset> mSavAsset;
    unordered_map<string, igmonitor::OpenOrder> mSpotOpenOrder;
    unordered_map<string, igmonitor::OpenOrder> mUFutureOpenOrder;
    unordered_map<string, igmonitor::OpenOrder> mCFutureOpenOrder;
    unordered_map<string, igmonitor::OpenOrder> mDeliveryOpenOrder;
    unordered_map<string, igmonitor::OpenOrder> mPerpetualOpenOrder;
    unordered_map<string, igmonitor::MarAsset> mMarAsset;
    unordered_map<string, igmonitor::Asset> mCrossMarAsset;
    double totalMarginBalance;
    unordered_map<string, igmonitor::PositionFundingRate> mPositionFundingRate;
    igmonitor::RiskInfo riskInfo;
    igmonitor::TotalMarAsset totalMarAsset;
    vector<igmonitor::LoBo> vLoBo;

    double initialMarginRate;
    double unifyMaintenanceMarginRate;

    string baseAsset;
    double MINDOUBLE;

    bool adapterQuery;
    vector<string> adapterQueryErrMsg;

    int triggerInterval;
    unordered_map<int64_t, double> mLeverageAlarm;
    unordered_map<int64_t, double> mRiskExposureAlarm;
    unordered_map<int64_t, double> mUnderwayOrderValueAlarm;
    AlarmInfo alarmInfo;

    bool maxLeverageUD;
    bool maxLeverageUS;
    string maxLeverageAssetD;
    string maxLeverageAssetS;
    string maxLeverageTabD;
    string maxLeverageTabS;
    string maxRiskExposureTab;
    string maxRiskExposureAssetD;
    string maxRiskExposureAssetS;
    
    double totalExposure;
};
