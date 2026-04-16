#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "sqlite3.h"
#include "Utility.h"

using namespace std;


class MonitorConfig {
public:
    static MonitorConfig& GetInstance();
    ~MonitorConfig();
    void LoadConfig();
    void LoadAlarmConfig();
    void LoadMdConfig();
    void LoadCoinbaseMdConfig();
    void LoadAssetRateConfig();
    void LoadConfigFromSqlite();
    string GetLogTag();
    int GetLogLevel();
    string GetLogPath();
    string GetMdAddr();
    int GetMdPort();
    string GetMdPassword();
    vector<string>& GetMdSubChannels();
    vector<string>& GetTradeSubChannels();
    string GetDbAddr();
    int GetDbPort();
    string GetDbPassword();
    string GetPubAddr();
    int GetPubPort();
    string GetPubPassword();
    string GetPhysicalPubChannel();
    string GetStrategyPubChannel();
    string GetOverviewPubChannel();
    string GetMdPubChannel();
    string GetMdStatusPubChannel();
    string GetParaChannel();
    unordered_map<int, AccountInfo>& GetAccountInfo();
    unordered_map<string, ProductInfo>& GetProductInfo();
    vector<AccountMonitorInfo>& GetAccountMonitorInfo();
    string GetBaseAssetById(int customerId);
    string GetProductBaseAsset(string name);
    int GetProductId(string name);
    vector<int>& GetStrategyAccount();
    string GetLarkUrl();
    string GetLarkAuthorization();
    int GetLarkChannel();
    //int GetLarkEnable();
    int GetTriggerInterval();
    double GetSystemAlarmTime();
    double GetMdAlarm();
    double GetMdKlineAlarm();
    double GetMdTradeAlarm();
    AlarmInfo& GetAlarmInfoById(int customerId);
    AlarmInfo& GetProductAlarmInfo(string name);
    vector<ReceiveInfo>& GetReceiveInfo();
    unordered_map<string, ReceiveGroupInfo>& GetReceiveGroupInfo();
    vector<string>& GetMdChannels();
    vector<string>& GetCoinbaseMdSymbol();
    string GetAccountNameByAccountId(int id);
    double GetOrderNetValuePercent();
    double GetOrderUsdtValueDown();
    double GetOrderUsdtValueUp();
    OrderValueThreshold& GetOrderValueThresholdTotal();
    unordered_map<int, OrderValueThreshold>& GetOrderValueThreshold();
    unordered_map<int, AccountVoiceCall>& GetAccountVoiceCall();
    unordered_map<string, AccountVoiceCall>& GetProductVoiceCall();
    vector<MdPriceVoiceCall>& GetMdPriceVoiceCall();
    void SetMdPriceVoiceCall(vector<MdPriceVoiceCall> v);
    double GetVoiceCallInterval();
    bool IsAccountIdInProduct(int id);
    double GetOpenInterestAlarm();

    string GetMysqlAddr();
    int GetMysqlPort();
    string GetMysqlUser();
    string GetMysqlPassword();
    string GetMysqlDbName();
    string GetMysqlOrdTableName();
    string GetMysqlOrdDetailTableName();
    string GetMysqlRiskInfoTableName();

    string GetMarketMysqlAddr();
    int GetMarketMysqlPort();
    string GetMarketMysqlUser();
    string GetMarketMysqlPassword();
    string GetMarketMysqlDbVolumeName();
    string GetMarketMysqlDbOpenInterestName();

    set<string> GetBinanceDisconnectSpot();
    set<string> GetBinanceDisconnectUfuture();
    set<string> GetBinanceDisconnectCfuture();
    set<string> GetGateioDisconnectSpot();
    set<string> GetGateioDisconnectSwap();
    string GetMdVoiceCallGroupId();

    double GetAssetRate(string exchangeStr, string asset);

    string GetAccountLarkUrl();
    string GetSystemLarkUrl();
    string GetExchangeLarkUrl();

private:
    MonitorConfig();

    string logTag;
    int logLevel;
    string logPath;

    string mdAddr;
    int mdPort;
    string mdPassword;

    string dbAddr;
    int dbPort;
    string dbPassword;

    string pubAddr;
    int pubPort;
    string pubPassword;
    string physicalPubChannel;
    string strategyPubChannel;
    string overviewPubChannel;
    string mdPubChannel;
    string mdStatusPubChannel;
    string paraChannel;

    unordered_map<int, AccountInfo> mAccountInfo;
    unordered_map<string, ProductInfo> mProductInfo;
    vector<int> vStrategyAccount;
    vector<string> vMdSubChannel;
    vector<AccountMonitorInfo> vAccountMonitorInfo;
    vector<string> vMdChannels;
    vector<string> vCoinbaseMdSymbol;
    unordered_map<string, double> mAssetRate;

    // alarm
    string alarmPath;
    string larkUrl;
    string larkAuthorization;
    int larkChannel;
    //int larkEnable;
    int triggerInterval;
    double systemAlarmTime;
    double mdAlarm;
    double mdKlineAlarm;
    double mdTradeAlarm;
    
    set<string> sBinanceDisconnectSpot;
    set<string> sBinanceDisconnectUfuture;
    set<string> sBinanceDisconnectCfuture;
    set<string> sGateioDisconnectSpot;
    set<string> sGateioDisconnectSwap;
    string mdVoiceCallGroupId;


    vector<int> vAccountId;
    unordered_map<int, AlarmInfo> mAlarmInfo;
    vector<ReceiveInfo> vReceiveInfo;
    unordered_map<string, ReceiveGroupInfo> mReceiveGroupInfo;
    // order
    double orderNetValuePercent;
    double orderUsdtValueDown;
    double orderUsdtValueUp;
    OrderValueThreshold orderValueThresholdTotal;
    unordered_map<int, OrderValueThreshold> mOrderValueThreshold;
    // voice call
    unordered_map<int, AccountVoiceCall> mAccountVoiceCall;
    vector<MdPriceVoiceCall> vMdPriceVoiceCall;
    double voiceCallInterval;
    unordered_map<string, AccountVoiceCall> mProductVoiceCall;

    vector<string> vProduct;
    unordered_map<string, AlarmInfo> mProductAlarmInfo;

    double openInterestAlarm;

    // mysql
    string mysqlAddr;
    int mysqlPort;
    string mysqlUser;
    string mysqlPassowrd;
    string mysqlDbName;
    string mysqlOrdName;
    string mysqlOrdDetailName;
    string mysqlRiskInfoName;

    // market mysql
    string marketMysqlAddr;
    int marketMysqlPort;
    string marketMysqlUser;
    string marketMysqlPassowrd;
    string marketMysqlDbVolume;
    string marketMysqlDbOpenInterest;

    string mdPath;
    string coinbaseMdPath;
    string assetRatePath;

    // new lark config
    string accountLarkUrl;
    string systemLarkUrl;
    string exchangeLarkUrl;
};
