#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "Utility.h"


class MonitorConfig {
public:
    static MonitorConfig& GetInstance();
    ~MonitorConfig();
    void LoadConfig();
    void LoadAlarmConfig();
    std::string GetLogTag();
    int GetLogLevel();
    std::string GetLogPath();
    std::string GetDbAddr();
    int GetDbPort();
    std::string GetDbPassword();
    std::string GetPubAddr();
    int GetPubPort();
    std::string GetPubPassword();
    std::string GetPhysicalPubChannel();
    std::string GetOverviewPubChannel();
    std::unordered_map<int, AccountInfo>& GetAccountInfo();
    std::unordered_map<std::string, ProductInfo>& GetProductInfo();
    std::string GetBaseAssetById(int customerId);
    std::string GetProductBaseAsset(std::string name);
    int GetProductId(std::string name);
    std::string GetLarkUrl();
    double GetSystemAlarmTime();
    AlarmInfo& GetAlarmInfoById(int customerId);
    AlarmInfo& GetProductAlarmInfo(std::string name);
    std::string GetAccountNameByAccountId(int id);
    bool IsAccountIdInProduct(int id);

    std::string GetMysqlAddr();
    int GetMysqlPort();
    std::string GetMysqlUser();
    std::string GetMysqlPassword();
    std::string GetMysqlDbName();
    std::string GetMysqlRiskInfoTableName();

    std::string GetAccountLarkUrl();
    std::string GetSystemLarkUrl();
    std::string GetExchangeLarkUrl();

private:
    MonitorConfig();

    std::string logTag;
    int logLevel;
    std::string logPath;

    std::string mdAddr;
    int mdPort;
    std::string mdPassword;

    std::string dbAddr;
    int dbPort;
    std::string dbPassword;

    std::string pubAddr;
    int pubPort;
    std::string pubPassword;
    std::string physicalPubChannel;
    std::string overviewPubChannel;

    std::unordered_map<int, AccountInfo> mAccountInfo;
    std::unordered_map<std::string, ProductInfo> mProductInfo;
   
    // alarm
    std::string alarmPath;
    std::string larkUrl;
    double systemAlarmTime;
    
    std::vector<int> vAccountId;
    std::unordered_map<int, AlarmInfo> mAlarmInfo;

    std::vector<std::string> vProduct;
    std::unordered_map<std::string, AlarmInfo> mProductAlarmInfo;

    // mysql
    std::string mysqlAddr;
    int mysqlPort;
    std::string mysqlUser;
    std::string mysqlPassowrd;
    std::string mysqlDbName;
    std::string mysqlRiskInfoName;

    // new lark config
    std::string accountLarkUrl;
    std::string systemLarkUrl;
    std::string exchangeLarkUrl;
};
