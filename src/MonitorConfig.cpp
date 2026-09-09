#include "MonitorConfig.h"

MonitorConfig::MonitorConfig() {
    mAccountInfo.clear();
}

MonitorConfig::~MonitorConfig() {
    mAccountInfo.clear();
    mProductInfo.clear();
    vAccountId.clear();
    mAlarmInfo.clear();
    vProduct.clear();
    mProductAlarmInfo.clear();
}

MonitorConfig& MonitorConfig::GetInstance() {
    static MonitorConfig monitorConfig;
    return monitorConfig;
}

void MonitorConfig::LoadConfig() {
    boost::property_tree::ptree properties;
	boost::property_tree::ini_parser::read_ini("monitor.ini", properties);
    
    auto itemSummary = properties.get_child("SUMMARY");
    logTag = itemSummary.get<string>("logtag");
    logLevel = itemSummary.get<int>("loglevel");
    logPath = itemSummary.get<string>("logpath");
    int accountCount = itemSummary.get<int>("accountcount");
    int productCount = itemSummary.get<int>("productcount");
    alarmPath = itemSummary.get<string>("alarmpath");
            
    auto itemDb = properties.get_child("DB");
    dbAddr = itemDb.get<string>("addr");
    dbPort = itemDb.get<int>("port");
    dbPassword = itemDb.get<string>("password");

    auto itemPub = properties.get_child("PUB");
    pubAddr = itemPub.get<string>("addr");
    pubPort = itemPub.get<int>("port");
    pubPassword = itemPub.get<string>("password");
    physicalPubChannel = itemPub.get<string>("physicalpubchannel");
    overviewPubChannel = itemPub.get<string>("overviewpubchannel");
    
    auto itemMysql = properties.get_child("MYSQL");
    mysqlAddr = itemMysql.get<string>("addr");
    mysqlPort = itemMysql.get<int>("port");
    mysqlUser = itemMysql.get<string>("user");
    mysqlPassowrd = itemMysql.get<string>("password");
    mysqlDbName = itemMysql.get<string>("dbname");
    mysqlRiskInfoName = itemMysql.get<string>("dbriskinfo", "");


    for (int i = 1; i <= accountCount; ++i) {
        AccountInfo accountInfo;
        string tag = "ACCOUNT" + to_string(i);
        auto itemAccount = properties.get_child(tag);
        accountInfo.accountName = itemAccount.get<string>("accountname");
        accountInfo.accountId = itemAccount.get<int>("accountid");
        accountInfo.exchangeType = (ExchangeType)itemAccount.get<int>("exchangetype");
        accountInfo.apiKey = itemAccount.get<string>("apikey");
        accountInfo.secretKey = itemAccount.get<string>("secretkey");
        accountInfo.passphrase = itemAccount.get<string>("passphrase", "");
        accountInfo.userId = itemAccount.get<string>("userId", "");
        accountInfo.restUrl = itemAccount.get<string>("resturl");
        accountInfo.wssUrl = itemAccount.get<string>("wssurl");
        accountInfo.baseAsset = itemAccount.get<string>("baseasset");
        accountInfo.apiPermission = itemAccount.get<string>("apipermission");
        accountInfo.hedge = itemAccount.get<int>("hedge", 1);
        accountInfo.unified = itemAccount.get<int>("unified", 0);  // 统一账户
        mAccountInfo[accountInfo.accountId] = accountInfo;
    }

    for (int i = 1; i <= productCount; ++i) {
        ProductInfo productInfo;
        string tag = "PRODUCT" + to_string(i);
        auto itemProduct = properties.get_child(tag);
        productInfo.productId = itemProduct.get<int>("productid", 0);
        productInfo.productName = itemProduct.get<string>("productname");
        productInfo.baseAsset = itemProduct.get<string>("baseasset");
        string accountId = itemProduct.get<string>("accountid");
        vector<string> v;
        SplitString(accountId, ",", v);
        for (size_t j = 0; j < v.size(); ++j) {
            int acountId = atoi(v[j].c_str());
            productInfo.vAccountId.push_back(acountId);
        }
        mProductInfo[productInfo.productName] = productInfo;
    } 

    LoadAlarmConfig();
}

void MonitorConfig::LoadAlarmConfig() {
    if (alarmPath.size() > 0) {
        boost::property_tree::ptree properties;
        boost::property_tree::ini_parser::read_ini(alarmPath, properties);
        
        auto itemSummary = properties.get_child("SUMMARY");
        int accountCount = itemSummary.get<int>("accountcount");
        int productCount = itemSummary.get<int>("productcount");
        larkUrl = itemSummary.get<string>("larkurl");

        accountLarkUrl = itemSummary.get<string>("accountlarkurl", "");
        systemLarkUrl = itemSummary.get<string>("systemlarkurl", "");
        exchangeLarkUrl = itemSummary.get<string>("exchangelarkurl", "");

        for (int i = 1; i <= accountCount; ++i) {
            AlarmInfo alarmInfo;
            string tag = "ACCOUNT" + to_string(i);
            auto itemAccount = properties.get_child(tag);
            int accountId = itemAccount.get<int>("accountid");

            mAlarmInfo[accountId] = alarmInfo;
            vAccountId.push_back(accountId);
        }
        
        for (int i = 1; i <= accountCount; ++i) {
            LeverageThreshold leverageThreshold;
            string tag = "LEVERAGE" + to_string(i);
            auto itemLeverage = properties.get_child(tag);
            leverageThreshold.warning = itemLeverage.get<double>("warning");
            leverageThreshold.alarm = itemLeverage.get<double>("alarm");
            int accountId = vAccountId[i - 1];
            mAlarmInfo[accountId].leverageThreshold = leverageThreshold;
        }

        for (int i = 1; i <= accountCount; ++i) {
            RiskExposureThreshold riskExposureThreshold;
            string tag = "RISKEXPOSURE" + to_string(i);
            auto itemRiskExposure = properties.get_child(tag);
            riskExposureThreshold.warning = itemRiskExposure.get<double>("warning");
            riskExposureThreshold.alarm = itemRiskExposure.get<double>("alarm");
            riskExposureThreshold.uwarning = itemRiskExposure.get<double>("uwarning", -1);
            riskExposureThreshold.ualarm = itemRiskExposure.get<double>("ualarm", -1);
            riskExposureThreshold.totalExposureWarning = itemRiskExposure.get<double>("totalexposurewarning", -1);
            riskExposureThreshold.totalExposureAlarm = itemRiskExposure.get<double>("totalexposurealarm", -1);
            string noWarningAsset = itemRiskExposure.get<string>("nowarningasset", "");
            vector<string> v;
            SplitString(noWarningAsset, ",", v);
            for (size_t k = 0; k < v.size(); k++) {
                riskExposureThreshold.sNoWarningAsset.insert(v[k]);
            }
            int accountId = vAccountId[i - 1];
            mAlarmInfo[accountId].riskExposureThreshold = riskExposureThreshold;
        }

        for (int i = 1; i <= accountCount; ++i) {
            UnderwayOrderValueThreshold underwayOrderValueThreshold;
            string tag = "UNDERWAYORDERVALUE" + to_string(i);
            auto itemUnderwayOrder = properties.get_child(tag);
            underwayOrderValueThreshold.warning = itemUnderwayOrder.get<double>("warning");
            underwayOrderValueThreshold.alarm = itemUnderwayOrder.get<double>("alarm");
            int accountId = vAccountId[i - 1];
            mAlarmInfo[accountId].underwayOrderValueThreshold = underwayOrderValueThreshold;
        }

        for (int i = 1; i <= accountCount; ++i) {
            NetValueThreshold netValueThreshold;
            string tag = "NETVALUE" + to_string(i);
            auto itemNetValue = properties.get_child(tag);
            netValueThreshold.monthValue = itemNetValue.get<double>("month", -1);
            netValueThreshold.initValue = itemNetValue.get<double>("init", -1);
            netValueThreshold.percent = itemNetValue.get<double>("percent", 1);
            int accountId = vAccountId[i - 1];
            mAlarmInfo[accountId].netValueThreshold = netValueThreshold;
        }

        for (int i = 1; i <= accountCount; ++i) {
            FundingRateThreshold fundingRateThreshold;
            string tag = "FUNDINGRATE" + to_string(i);
            auto itemFundingRate = properties.get_child(tag);
            fundingRateThreshold.alarm = itemFundingRate.get<double>("alarm");
            int accountId = vAccountId[i - 1];
            mAlarmInfo[accountId].fundingRateThreshold = fundingRateThreshold;
        }

        for (int i = 1; i <= accountCount; ++i) {
            LiquidationPriceThreshold liquidationPriceThreshold;
            string tag = "LIQUIDATIONPRICE" + to_string(i);
            auto itemLiquidationPrice = properties.get_child(tag);
            liquidationPriceThreshold.warning = itemLiquidationPrice.get<double>("warning");
            liquidationPriceThreshold.alarm = itemLiquidationPrice.get<double>("alarm");
            int accountId = vAccountId[i - 1];
            mAlarmInfo[accountId].liquidationPriceThreshold = liquidationPriceThreshold;
        }

        for (int i = 1; i <= accountCount; ++i) {
            MarginRateThreshold marginRateThreshold;
            string tag = "MARGINRATE" + to_string(i);
            auto itemMarginRate = properties.get_child(tag);
            marginRateThreshold.unifyMaintenanceMarginRateWarning = itemMarginRate.get<double>("unifymaintenancemarginratewarning", 0);
            marginRateThreshold.unifyMaintenanceMarginRateAlarm = itemMarginRate.get<double>("unifymaintenancemarginratealarm", 0);
            int accountId = vAccountId[i - 1];
            mAlarmInfo[accountId].marginRateThreshold = marginRateThreshold;
        }

        for (int i = 1; i <= productCount; ++i) {
            AlarmInfo alarmInfo;
            string tag = "PRODUCT" + to_string(i);
            auto itemProduct = properties.get_child(tag);
            string productName = itemProduct.get<string>("productname");
            mProductAlarmInfo[productName] = alarmInfo;
            vProduct.push_back(productName);
        }

        for (int i = 1; i <= productCount; ++i) {
            LeverageThreshold leverageThreshold;
            string tag = "PRODUCTLEVERAGE" + to_string(i);
            auto itemLeverage = properties.get_child(tag);
            leverageThreshold.warning = itemLeverage.get<double>("warning");
            leverageThreshold.alarm = itemLeverage.get<double>("alarm");
            string productName = vProduct[i - 1];
            mProductAlarmInfo[productName].leverageThreshold = leverageThreshold;
        }

        for (int i = 1; i <= productCount; ++i) {
            RiskExposureThreshold riskExposureThreshold;
            string tag = "PRODUCTRISKEXPOSURE" + to_string(i);
            auto itemRiskExposure = properties.get_child(tag);
            riskExposureThreshold.warning = itemRiskExposure.get<double>("warning");
            riskExposureThreshold.alarm = itemRiskExposure.get<double>("alarm");
            string productName = vProduct[i - 1];
            mProductAlarmInfo[productName].riskExposureThreshold = riskExposureThreshold;
        }

        for (int i = 1; i <= productCount; ++i) {
            UnderwayOrderValueThreshold underwayOrderValueThreshold;
            string tag = "PRODUCTUNDERWAYORDERVALUE" + to_string(i);
            auto itemRiskExposure = properties.get_child(tag);
            underwayOrderValueThreshold.warning = itemRiskExposure.get<double>("warning");
            underwayOrderValueThreshold.alarm = itemRiskExposure.get<double>("alarm");
            string productName = vProduct[i - 1];
            mProductAlarmInfo[productName].underwayOrderValueThreshold = underwayOrderValueThreshold;
        }

        for (int i = 1; i <= productCount; ++i) {
            NetValueThreshold netValueThreshold;
            string tag = "PRODUCTNETVALUE" + to_string(i);
            auto itemNetValue = properties.get_child(tag);
            netValueThreshold.monthValue = itemNetValue.get<double>("month", -1);
            netValueThreshold.initValue = itemNetValue.get<double>("init", -1);
            netValueThreshold.percent = itemNetValue.get<double>("percent", 1);
            string productName = vProduct[i - 1];
            mProductAlarmInfo[productName].netValueThreshold = netValueThreshold;
        }

        for (int i = 1; i <= productCount; ++i) {
            FundingRateThreshold fundingRateThreshold;
            string tag = "PRODUCTFUNDINGRATE" + to_string(i);
            auto itemFundingRate = properties.get_child(tag);
            fundingRateThreshold.alarm = itemFundingRate.get<double>("alarm");
            string productName = vProduct[i - 1];
            mProductAlarmInfo[productName].fundingRateThreshold = fundingRateThreshold;
        }

        auto itemSystem = properties.get_child("SYSTEM");
        systemAlarmTime = itemSystem.get<double>("time");
    }
}

string MonitorConfig::GetLogTag() {
    return logTag;
}

int MonitorConfig::GetLogLevel() {
    return logLevel;
}

string MonitorConfig::GetLogPath() {
    return logPath;
}

string MonitorConfig::GetDbAddr() {
    return dbAddr;
}

int MonitorConfig::GetDbPort() {
    return dbPort;
}

string MonitorConfig::GetDbPassword() {
    return dbPassword;
}

string MonitorConfig::GetPubAddr() {
    return pubAddr;
}

int MonitorConfig::GetPubPort() {
    return pubPort;
}

string MonitorConfig::GetPubPassword() {
    return pubPassword;
}

string MonitorConfig::GetPhysicalPubChannel() {
    return physicalPubChannel;
}

string MonitorConfig::GetOverviewPubChannel() {
    return overviewPubChannel;
}

unordered_map<int, AccountInfo>& MonitorConfig::GetAccountInfo() {
    return mAccountInfo;
}

unordered_map<string, ProductInfo>& MonitorConfig::GetProductInfo() {
    return mProductInfo;
}

string MonitorConfig::GetBaseAssetById(int customerId) {
    string baseAsset = "USDT";
    auto iter = mAccountInfo.find(customerId);
    if (iter != mAccountInfo.end()) {
        baseAsset = iter->second.baseAsset;
    }
    return baseAsset;
}

string MonitorConfig::GetProductBaseAsset(string name) {
    string baseAsset = "USDT";
    auto iter = mProductInfo.find(name);
    if (iter != mProductInfo.end()) {
        baseAsset = iter->second.baseAsset;
    }
    return baseAsset;
}

int MonitorConfig::GetProductId(string name) {
    int productId = 0;
    auto iter = mProductInfo.find(name);
    if (iter != mProductInfo.end()) {
        productId = iter->second.productId;
    }
    return productId;
}

string MonitorConfig::GetLarkUrl() {
    return larkUrl;
}

double MonitorConfig::GetSystemAlarmTime() {
    return systemAlarmTime;
}

AlarmInfo& MonitorConfig::GetAlarmInfoById(int customerId) {
    return mAlarmInfo[customerId];
}

AlarmInfo& MonitorConfig::GetProductAlarmInfo(string name) {
    return mProductAlarmInfo[name];
}

string MonitorConfig::GetAccountNameByAccountId(int id) {
    string name = "";
    auto iter = mAccountInfo.find(id);
    if (iter != mAccountInfo.end()) {
        name = iter->second.accountName;
    } else {
        string idStr = to_string(id);
        if (idStr.size() >= 4) {
            string accId = idStr.substr(0, 4);
            for (auto it = mAccountInfo.begin(); it != mAccountInfo.end(); ++it) {
                string accountIdStr = to_string(it->first);
                if (accountIdStr.size() >= 4) {
                    string accountId = accountIdStr.substr(0, 4);
                    if (accId == accountId) {
                        name = it->second.accountName;
                        break;
                    }
                }
            }
        }
    }
    return name;
}

bool MonitorConfig::IsAccountIdInProduct(int id) {
    bool exist = false;
    for (auto iter = mProductInfo.begin(); iter != mProductInfo.end(); ++iter) {
    	for (size_t i = 0; i < iter->second.vAccountId.size(); ++i) {
	    if (id == iter->second.vAccountId[i]) {
	    	exist = true;
		break;
	    }
	}
	if (exist) {
	    break;
	}
    }
    return exist;
}

string MonitorConfig::GetMysqlAddr() {
    return mysqlAddr;
}

int MonitorConfig::GetMysqlPort() {
    return mysqlPort;
}

string MonitorConfig::GetMysqlUser() {
    return mysqlUser;
}

string MonitorConfig::GetMysqlPassword() {
    return mysqlPassowrd;
}

string MonitorConfig::GetMysqlDbName() {
    return mysqlDbName;
}

string MonitorConfig::GetMysqlRiskInfoTableName() {
    return mysqlRiskInfoName;
}
    
string MonitorConfig::GetAccountLarkUrl() {
    return accountLarkUrl;
}

string MonitorConfig::GetSystemLarkUrl() {
    return systemLarkUrl;
}

string MonitorConfig::GetExchangeLarkUrl() {
    return exchangeLarkUrl;
}