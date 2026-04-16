#include "MonitorConfig.h"

MonitorConfig::MonitorConfig() {
    mAccountInfo.clear();
    vStrategyAccount.clear();
    vMdSubChannel.clear();
    larkChannel = 1;
}

MonitorConfig::~MonitorConfig() {
    mAccountInfo.clear();
    mProductInfo.clear();
    vStrategyAccount.clear();
    vMdSubChannel.clear();
    vAccountMonitorInfo.clear();
    vMdChannels.clear();
    vCoinbaseMdSymbol.clear();
    vAccountId.clear();
    mAlarmInfo.clear();
    vReceiveInfo.clear();
    mReceiveGroupInfo.clear();
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
    int tradeCount = itemSummary.get<int>("tradecount");
    alarmPath = itemSummary.get<string>("alarmpath");
    mdPath = itemSummary.get<string>("mdpath", "");
    coinbaseMdPath = itemSummary.get<string>("coinbasemdpath", "");
    assetRatePath = itemSummary.get<string>("assetratepath", "");

    auto itemStrategyAccount = properties.get_child("STRATEGYACCOUNT");
    string strategyAccount = itemStrategyAccount.get<string>("accountid", "");
    vector<string> v;
    SplitString(strategyAccount, ",", v);
    for (size_t i = 0; i < v.size(); ++i) {
        int accountId = atoi(v[i].c_str());
        vStrategyAccount.push_back(accountId);
    }

	auto itemMd = properties.get_child("MD");
    mdAddr = itemMd.get<string>("addr");
    mdPort = itemMd.get<int>("port");
    mdPassword = itemMd.get<string>("password");
    string mdSubChannels = itemMd.get<string>("subchannels");
    SplitString(mdSubChannels, ",", vMdSubChannel);
            
    auto itemDb = properties.get_child("DB");
    dbAddr = itemDb.get<string>("addr");
    dbPort = itemDb.get<int>("port");
    dbPassword = itemDb.get<string>("password");

    auto itemPub = properties.get_child("PUB");
    pubAddr = itemPub.get<string>("addr");
    pubPort = itemPub.get<int>("port");
    pubPassword = itemPub.get<string>("password");
    physicalPubChannel = itemPub.get<string>("physicalpubchannel");
    strategyPubChannel = itemPub.get<string>("strategypubchannel");
    overviewPubChannel = itemPub.get<string>("overviewpubchannel");
    mdPubChannel = itemPub.get<string>("mdpubchannel");
    mdStatusPubChannel = itemPub.get<string>("mdstatuspubchannel", "");
    paraChannel = itemPub.get<string>("parachannel", "");
    
    auto itemMysql = properties.get_child("MYSQL");
    mysqlAddr = itemMysql.get<string>("addr");
    mysqlPort = itemMysql.get<int>("port");
    mysqlUser = itemMysql.get<string>("user");
    mysqlPassowrd = itemMysql.get<string>("password");
    mysqlDbName = itemMysql.get<string>("dbname");
    mysqlOrdName = itemMysql.get<string>("dborder");
    mysqlOrdDetailName = itemMysql.get<string>("dborderdetail");
    mysqlRiskInfoName = itemMysql.get<string>("dbriskinfo", "");

    auto itemMarketMysql = properties.get_child("MARKETMYSQL");
    marketMysqlAddr = itemMarketMysql.get<string>("addr");
    marketMysqlPort = itemMarketMysql.get<int>("port");
    marketMysqlUser = itemMarketMysql.get<string>("user");
    marketMysqlPassowrd = itemMarketMysql.get<string>("password");
    marketMysqlDbVolume = itemMarketMysql.get<string>("dbvolume");
    marketMysqlDbOpenInterest = itemMarketMysql.get<string>("dbopeninterest");

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

    for (int i = 1; i <= tradeCount; ++i) {
        AccountMonitorInfo accountMonitorInfo;
        string tag = "TRADE" + to_string(i);
        auto itemTrade = properties.get_child(tag);
        accountMonitorInfo.addr = itemTrade.get<string>("addr");
        accountMonitorInfo.port = itemTrade.get<int>("port");
        accountMonitorInfo.password = itemTrade.get<string>("password");
        accountMonitorInfo.channels = itemTrade.get<string>("subchannels");
        vAccountMonitorInfo.push_back(accountMonitorInfo);
    }    

    LoadAlarmConfig();
    LoadMdConfig();
    LoadCoinbaseMdConfig();
    LoadAssetRateConfig();
}

void MonitorConfig::LoadAlarmConfig() {
    if (alarmPath.size() > 0) {
        boost::property_tree::ptree properties;
        boost::property_tree::ini_parser::read_ini(alarmPath, properties);
        
        auto itemSummary = properties.get_child("SUMMARY");
        int accountCount = itemSummary.get<int>("accountcount");
        int productCount = itemSummary.get<int>("productcount");
        int reciveCount = itemSummary.get<int>("receivecount");
        int mdPriceCount = itemSummary.get<int>("mdpricecount", 0);
        voiceCallInterval = itemSummary.get<int>("voicecallinterval", 60);
        larkUrl = itemSummary.get<string>("larkurl");
        larkAuthorization = itemSummary.get<string>("larkauthorization");
        larkChannel = itemSummary.get<int>("larkchannel", 1);
        //larkEnable = itemSummary.get<int>("larkenable");
        triggerInterval = itemSummary.get<int>("trigger");

        accountLarkUrl = itemSummary.get<string>("accountlarkurl", "");
        systemLarkUrl = itemSummary.get<string>("systemlarkurl", "");
        exchangeLarkUrl = itemSummary.get<string>("exchangelarkurl", "");

        for (int i = 1; i <= accountCount; ++i) {
            AlarmInfo alarmInfo;
            string tag = "ACCOUNT" + to_string(i);
            auto itemAccount = properties.get_child(tag);
            int accountId = itemAccount.get<int>("accountid");
            string groupId = itemAccount.get<string>("voicecallgroupid", "");
            string userId = itemAccount.get<string>("voicecalluserid", "");
            string userIdNetValue = itemAccount.get<string>("voicecalluseridnetvalue", "");
            string userIdLiquidationPrice = itemAccount.get<string>("voicecalluseridliquidationprice", "");

            mAlarmInfo[accountId] = alarmInfo;
            vAccountId.push_back(accountId);

            AccountVoiceCall accountVoiceCall;
            accountVoiceCall.groupId = groupId;
            vector<string> v;
            SplitString(userId, ",", v);
            vector<string> vNetValue;
            SplitString(userIdNetValue, ",", vNetValue);
            vector<string> vLiquidationPrice;
            SplitString(userIdLiquidationPrice, ",", vLiquidationPrice);
            accountVoiceCall.vUserId = v;
            accountVoiceCall.vUserIdNetValue = vNetValue;
            accountVoiceCall.vUserIdLiquidationPrice = vLiquidationPrice;
            mAccountVoiceCall[accountId] = accountVoiceCall;
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
            marginRateThreshold.initialMarginRateWarning = itemMarginRate.get<double>("initmarginratewarning", 0);
            marginRateThreshold.initialMarginRateAlarm = itemMarginRate.get<double>("initmarginratealarm", 0);
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

            string groupId = itemProduct.get<string>("voicecallgroupid", "");
            string userId = itemProduct.get<string>("voicecalluserid", "");
            string userIdNetValue = itemProduct.get<string>("voicecalluseridnetvalue", "");
            AccountVoiceCall accountVoiceCall;
            accountVoiceCall.groupId = groupId;
            vector<string> v;
            SplitString(userId, ",", v);
            vector<string> vNetValue;
            SplitString(userIdNetValue, ",", vNetValue);
            accountVoiceCall.vUserId = v;
            accountVoiceCall.vUserIdNetValue = vNetValue;
            mProductVoiceCall[productName] = accountVoiceCall;
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

        auto itemMd = properties.get_child("MD");
        mdAlarm = itemMd.get<double>("alarm");
        mdKlineAlarm = itemMd.get<double>("klinealarm", 0);
        mdTradeAlarm = itemMd.get<double>("tradealarm", 0);


        string binanceDisconnectSpot = itemMd.get<string>("binancedisconnectspot", "");
        vector<string> vBinanceSpot;
        SplitString(binanceDisconnectSpot, ",", vBinanceSpot);
        for (size_t i = 0; i < vBinanceSpot.size(); ++i) {
            sBinanceDisconnectSpot.insert(vBinanceSpot[i]);
        }

        string binanceDisconnectUfuture = itemMd.get<string>("binancedisconnectufuture", "");
        vector<string> vBinanceUfuture;
        SplitString(binanceDisconnectUfuture, ",", vBinanceUfuture);
        for (size_t i = 0; i < vBinanceUfuture.size(); ++i) {
            sBinanceDisconnectUfuture.insert(vBinanceUfuture[i]);
        }

        string binanceDisconnectCfuture = itemMd.get<string>("binancedisconnectcfuture", "");
        vector<string> vBinanceCfuture;
        SplitString(binanceDisconnectCfuture, ",", vBinanceCfuture);
        for (size_t i = 0; i < vBinanceCfuture.size(); ++i) {
            sBinanceDisconnectCfuture.insert(vBinanceCfuture[i]);
        }

        string gateioDisconnectSpot = itemMd.get<string>("gateiodisconnectspot", "");
        vector<string> vGateioSpot;
        SplitString(gateioDisconnectSpot, ",", vGateioSpot);
        for (size_t i = 0; i < vGateioSpot.size(); ++i) {
            sGateioDisconnectSpot.insert(vGateioSpot[i]);
        }

        string gateioDisconnectSwap = itemMd.get<string>("gateiodisconnectswap", "");
        vector<string> vGateioSwap;
        SplitString(gateioDisconnectSwap, ",", vGateioSwap);
        for (size_t i = 0; i < vGateioSwap.size(); ++i) {
            sGateioDisconnectSwap.insert(vGateioSwap[i]);
        }

        mdVoiceCallGroupId = itemMd.get<string>("voicegroupid", "");

        auto itemOrder = properties.get_child("ORDER");
        int accCount = itemOrder.get<int>("accountcount");
        orderValueThresholdTotal.orderNetValuePercent = itemOrder.get<double>("netvaluepercent", -1.0);
        orderValueThresholdTotal.orderUsdtValueDown = itemOrder.get<double>("usdtvaluedown", -1.0);
        orderValueThresholdTotal.orderUsdtValueUp = itemOrder.get<double>("usdtvalueup", -1.0);

        auto itemMarketInfo = properties.get_child("MARKETINFO");
        openInterestAlarm = itemMarketInfo.get<double>("openinterestalarm", -1);

        for (int i = 1; i <= accCount; ++i) {
            OrderValueThreshold orderValueThreshold;
            string accountIdChan = string("accountid") + to_string(i);
            int accountId = itemOrder.get<int>(accountIdChan);

            string netValueChan = string("netvaluepercent") + to_string(i);
            orderValueThreshold.orderNetValuePercent = itemOrder.get<double>(netValueChan, -1.0);

            string usdtValueDownChan = string("usdtvaluedown") + to_string(i);
            orderValueThreshold.orderUsdtValueDown = itemOrder.get<double>(usdtValueDownChan, -1.0);

            string usdtValueUpChan = string("usdtvalueup") + to_string(i);
            orderValueThreshold.orderUsdtValueUp = itemOrder.get<double>(usdtValueUpChan, -1.0);

            mOrderValueThreshold[accountId] = orderValueThreshold;
        }

        for (int i = 1; i <= reciveCount; ++i) {
            ReceiveGroupInfo receiveGroupInfo;
            string tag = "RECEIVE" + to_string(i);
            auto itemReceive = properties.get_child(tag);
            receiveGroupInfo.code = itemReceive.get<string>("code");
            receiveGroupInfo.importance = itemReceive.get<string>("importance");
            mReceiveGroupInfo[receiveGroupInfo.code] = receiveGroupInfo;
        }

        for (int i = 1; i <= mdPriceCount; ++i) {
            MdPriceVoiceCall mdPriceVoiceCall;
            string tag = "MDPRICE" + to_string(i);
            auto itemMdPrice = properties.get_child(tag);
            mdPriceVoiceCall.symbol = itemMdPrice.get<string>("symbol");
            mdPriceVoiceCall.price = itemMdPrice.get<double>("price");
            mdPriceVoiceCall.mode = itemMdPrice.get<int>("mode");
            string userId = itemMdPrice.get<string>("voicecalluserid");
            vector<string> v;
            SplitString(userId, ",", v);
            mdPriceVoiceCall.vUserId = v;
            vMdPriceVoiceCall.push_back(mdPriceVoiceCall);
        }
    }
}

void MonitorConfig::LoadMdConfig() {
    if (mdPath.size() > 0) {
        boost::property_tree::ptree properties;
	    boost::property_tree::ini_parser::read_ini(mdPath, properties);
    
        auto itemSummary = properties.get_child("SUMMARY");
        int channelCount = itemSummary.get<int>("channelcount");

        for (int i = 1; i <= channelCount; ++i) {
            string channel = string("channel") + to_string(i);
            string value = itemSummary.get<string>(channel);
            vMdChannels.push_back(value);
        }
    }
}

void MonitorConfig::LoadCoinbaseMdConfig() {
    if (coinbaseMdPath.size() > 0) {
        boost::property_tree::ptree properties;
	    boost::property_tree::ini_parser::read_ini(coinbaseMdPath, properties);
    
        auto itemSummary = properties.get_child("SUMMARY");
        int symbolCount = itemSummary.get<int>("symbolcount");

        for (int i = 1; i <= symbolCount; ++i) {
            string symbol = string("symbol") + to_string(i);
            string value = itemSummary.get<string>(symbol);
            vCoinbaseMdSymbol.push_back(value);
        }
    }
}

void MonitorConfig::LoadAssetRateConfig() {
    if (assetRatePath.size() > 0) {
        boost::property_tree::ptree properties;
	    boost::property_tree::ini_parser::read_ini(assetRatePath, properties);
    
        auto itemSummary = properties.get_child("SUMMARY");
        int assetRateCount = itemSummary.get<int>("assetratecount");

        for (int i = 1; i <= assetRateCount; ++i) {
            string channel = string("assetrate") + to_string(i);
            string value = itemSummary.get<string>(channel);
            vector<string> v;
            SplitString(value, ",", v);
            if (v.size() >= 2) {
                string asset = v[0];
                double rate = stod(v[1]);
                mAssetRate[asset] = rate;
            }
        }
    }
}

void MonitorConfig::LoadConfigFromSqlite() {
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

string MonitorConfig::GetMdAddr() {
    return mdAddr;
}

int MonitorConfig::GetMdPort() {
    return mdPort;
}

string MonitorConfig::GetMdPassword() {
    return mdPassword;
}

vector<string>& MonitorConfig::GetMdSubChannels() {
    return vMdSubChannel;
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

string MonitorConfig::GetStrategyPubChannel() {
    return strategyPubChannel;
}

string MonitorConfig::GetOverviewPubChannel() {
    return overviewPubChannel;
}

string MonitorConfig::GetMdPubChannel() {
    return mdPubChannel;
}

string MonitorConfig::GetMdStatusPubChannel() {
    return mdStatusPubChannel;
}

string MonitorConfig::GetParaChannel() {
    return paraChannel;
}

unordered_map<int, AccountInfo>& MonitorConfig::GetAccountInfo() {
    return mAccountInfo;
}

unordered_map<string, ProductInfo>& MonitorConfig::GetProductInfo() {
    return mProductInfo;
}

vector<AccountMonitorInfo>& MonitorConfig::GetAccountMonitorInfo() {
    return vAccountMonitorInfo;
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

vector<int>& MonitorConfig::GetStrategyAccount() {
    return vStrategyAccount;
}

string MonitorConfig::GetLarkUrl() {
    return larkUrl;
}

string MonitorConfig::GetLarkAuthorization() {
    return larkAuthorization;
}

int MonitorConfig::GetLarkChannel() {
    return larkChannel;
}

int MonitorConfig::GetTriggerInterval() {
    return triggerInterval;
}

double MonitorConfig::GetSystemAlarmTime() {
    return systemAlarmTime;
}

double MonitorConfig::GetMdAlarm() {
    return mdAlarm;
}

double MonitorConfig::GetMdKlineAlarm() {
    return mdKlineAlarm;
}

double MonitorConfig::GetMdTradeAlarm() {
    return mdTradeAlarm;
}

AlarmInfo& MonitorConfig::GetAlarmInfoById(int customerId) {
    return mAlarmInfo[customerId];
}

AlarmInfo& MonitorConfig::GetProductAlarmInfo(string name) {
    return mProductAlarmInfo[name];
}

vector<ReceiveInfo>& MonitorConfig::GetReceiveInfo() {
    return vReceiveInfo;
}

unordered_map<string, ReceiveGroupInfo>& MonitorConfig::GetReceiveGroupInfo() {
    return mReceiveGroupInfo;
}

vector<string>& MonitorConfig::GetMdChannels() {
    return vMdChannels;
}

vector<string>& MonitorConfig::GetCoinbaseMdSymbol() {
    return vCoinbaseMdSymbol;
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

double MonitorConfig::GetOrderNetValuePercent() {
    return orderNetValuePercent;
}

double MonitorConfig::GetOrderUsdtValueDown() {
    return orderUsdtValueDown;
}

double MonitorConfig::GetOrderUsdtValueUp() {
    return orderUsdtValueUp;
}

OrderValueThreshold& MonitorConfig::GetOrderValueThresholdTotal() {
    return orderValueThresholdTotal;
}

unordered_map<int, OrderValueThreshold>& MonitorConfig::GetOrderValueThreshold() {
    return mOrderValueThreshold;
}

unordered_map<int, AccountVoiceCall>& MonitorConfig::GetAccountVoiceCall() {
    return mAccountVoiceCall;
}

unordered_map<string, AccountVoiceCall>& MonitorConfig::GetProductVoiceCall() {
    return mProductVoiceCall;
}

vector<MdPriceVoiceCall>& MonitorConfig::GetMdPriceVoiceCall() {
    return vMdPriceVoiceCall;
}

void MonitorConfig::SetMdPriceVoiceCall(vector<MdPriceVoiceCall> v) {
    for (size_t i = 0; i < vMdPriceVoiceCall.size(); ++i) {
        for (size_t j = 0; j < v.size(); ++j) {
            if (vMdPriceVoiceCall[i].symbol == v[j].symbol) {
                vMdPriceVoiceCall[i].price = v[j].price;
                vMdPriceVoiceCall[i].mode = v[j].mode;
            }
        }
    }
}

double MonitorConfig::GetVoiceCallInterval() {
    return voiceCallInterval;
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

double MonitorConfig::GetOpenInterestAlarm() {
    return openInterestAlarm;
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

string MonitorConfig::GetMysqlOrdTableName() {
    return mysqlOrdName;
}

string MonitorConfig::GetMysqlOrdDetailTableName() {
    return mysqlOrdDetailName;
}

string MonitorConfig::GetMysqlRiskInfoTableName() {
    return mysqlRiskInfoName;
}

string MonitorConfig::GetMarketMysqlAddr() {
    return marketMysqlAddr;
}

int MonitorConfig::GetMarketMysqlPort() {
    return marketMysqlPort;
}

string MonitorConfig::GetMarketMysqlUser() {
    return marketMysqlUser;
}
    
string MonitorConfig::GetMarketMysqlPassword() {
    return marketMysqlPassowrd;
}

string MonitorConfig::GetMarketMysqlDbVolumeName() {
    return marketMysqlDbVolume;
}

string MonitorConfig::GetMarketMysqlDbOpenInterestName() {
    return marketMysqlDbOpenInterest;
}

set<string> MonitorConfig::GetBinanceDisconnectSpot() {
    return sBinanceDisconnectSpot;
}

set<string> MonitorConfig::GetBinanceDisconnectUfuture() {
    return sBinanceDisconnectUfuture;
}

set<string> MonitorConfig::GetBinanceDisconnectCfuture() {
    return sBinanceDisconnectCfuture;
}

set<string> MonitorConfig::GetGateioDisconnectSpot() {
    return sGateioDisconnectSpot;
}
    
set<string> MonitorConfig::GetGateioDisconnectSwap() {
    return sGateioDisconnectSwap;
}

string MonitorConfig::GetMdVoiceCallGroupId() {
    return mdVoiceCallGroupId;
}

double MonitorConfig::GetAssetRate(string exchangeStr, string asset) {
    double rate = 0.0;
    string key = exchangeStr + "." + asset;
    auto iter = mAssetRate.find(key);
    if (iter != mAssetRate.end()) {
        rate = iter->second;
    }

    return rate;
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