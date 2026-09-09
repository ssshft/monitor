#include "MonitorOperation.h"
#include "GateioAdapterMgr.h"
#include "BybitAdapterMgr.h"
#include "OkxAdapterMgr.h"
#include "MdMgr.h"
#include "ProductMgr.h"
#include "BinanceAdapterMgr.h"
#include "MsgPub.h"
#include "AccountMgr.h"
#include "MdMgr.h"
#include "AccountMonitor.h"
#include "LarkRebot.h"
#include "DataBase.h"

void MonitorOperation::RunAccountMonitor() {
    while (1) {
        int64_t currentTime = crypto::getCurrentTime();
  
        if (currentTime - lastCalcAccountStatsTime > calcAccountStatsInterval * 1000000LL) {
            try {
                std::unordered_map<std::string, std::string>& m = AccountMonitor::GetInstance().GetCurrentStatus(); // 使用从交易所查询的数据进行计算，权益，敞口，净值等

                // publish 账户信息
                std::string physicalPubChannel = MonitorConfig::GetInstance().GetPhysicalPubChannel();
                std::string overviewPubChannel = MonitorConfig::GetInstance().GetOverviewPubChannel();
                auto iter = m.find("physical");
                if (iter != m.end() && iter->second.length() > 0) {    
                    MsgPub::GetInstance().Publish(physicalPubChannel, iter->second);
                }

                iter = m.find("overview");
                if (iter != m.end() && iter->second.length() > 0) {
                    MsgPub::GetInstance().Publish(overviewPubChannel, iter->second);            
                }
            } catch(exception e) {
                std::cerr << "GetAccountStatus Error: " << e.what() << std::endl;
            }

            lastCalcAccountStatsTime = currentTime;
        }
    	
        
        // 报警
        if (sendAccountAlarm) {
            if (currentTime - lastAccountAlarmTime > accountAlarmInterval * 1000000LL) {
                try {
                    std::string accountUrl = MonitorConfig::GetInstance().GetAccountLarkUrl();
                    std::string exchangeUrl = MonitorConfig::GetInstance().GetExchangeLarkUrl();
                    std::vector<MsgCard> vAlarmMsg = AccountMonitor::GetInstance().GetAlarmMsg();  // 使用计算好的指标，判断是否需要报警
                    for (size_t j = 0; j < vAlarmMsg.size(); ++j) {
                        MsgCard msg = vAlarmMsg[j];
                        if (msg.title != "QueryAccount") {
                            if (msg.title == "TWAP-ADL") {
                                LarkRebot::GetInstance().SendLarkMsg(msg, accountUrl);
                            } 
                            else { // need alarm twice will send alarm msg
                                std::string key = "";
                                if (msg.name.length() > 0) {
                                    key = msg.name + "_" + msg.title;
                                } else {
                                    key = to_string(msg.accountId) + "_" + msg.title;
                                }

                                auto it = mAccountAlarmSendTime.find(key);
                                if (it != mAccountAlarmSendTime.end()) {
                                    if (currentTime - it->second > 6 * 60 * 1000000LL) {
                                        mAccountAlarmSendTime[key] = currentTime;
                                        continue;
                                    } else {
                                        mAccountAlarmSendTime[key] = currentTime;
                                    } 
                                } else {
                                    mAccountAlarmSendTime[key] = currentTime;
                                    continue;
                                }
                    
                                LarkRebot::GetInstance().SendLarkMsg(msg, accountUrl);
                            }
                        } else {
                            LarkRebot::GetInstance().SendLarkMsg(msg, exchangeUrl);
                        }
                    }
                } catch(exception e) {
                    std::cerr << "AccountAlarm Error: " << e.what() << std::endl;
                }

                lastAccountAlarmTime = currentTime;
            }
        }

        // 账户权益净值数据落库
        if (storeRiskInfo) {
            if (currentTime - lastStoreRiskInfoTime > storeRiskInfoInterval * 1000000LL) {
                try {
                    DataBase db;
                    std::vector<igmonitor::RiskInfo> vRiskInfo = AccountMgr::GetInstance().GetRiskInfo(); // 将计算好的指标写入数据库
                    db.InsertRiskInfo(vRiskInfo);

                    std::vector<igmonitor::RiskInfo> vProductRiskInfo = ProductMgr::GetInstance().GetRiskInfo();
                    db.InsertRiskInfo(vProductRiskInfo);
                } catch(exception e) {
                    std::cerr << "StoreRiskInfo Error: " << e.what() << std::endl;
                }

                lastStoreRiskInfoTime = currentTime;
            }
        }


        // 系统监控
        if (currentTime - lastSystemMonitorTime > systemMonitorInterval * 1000000LL) {
            try {
                std::string currentTimeStr = CovertToUtcStr(currentTime, false);  
                MsgCard msgCard;
                msgCard.templateId = 1;
                msgCard.title = "Systemtime";
                msgCard.object = "账户：风控系统";
                msgCard.datetime = currentTimeStr;
                msgCard.content = "风控系统运行中···";

                std::string systemUrl = MonitorConfig::GetInstance().GetSystemLarkUrl();
                LarkRebot::GetInstance().SendLarkMsg(msgCard, systemUrl);
            } catch(exception e) {
                std::cerr << "Systemtime Error: " << e.what() << std::endl;
            }

            lastSystemMonitorTime = currentTime;
        }

        if (currentTime - startTime > 5 * 60 * 1000000LL) { // 启动5分钟后，开始报警和落库，防止刚启动时数据不全导致错误的报警
            sendAccountAlarm = true;
            storeRiskInfo = true;
        }

    	usleep(1000);
    }
}

void MonitorOperation::UpdateAccountInfo() {
    while (1) {
	    try {
	        ProductMgr::GetInstance().UpdateAccountInfo();
            BinanceAdapterMgr::GetInstance().UpdateAccountInfo();  // query account info
            GateioAdapterMgr::GetInstance().UpdateAccountInfo();
            BybitAdapterMgr::GetInstance().UpdateAccountInfo();
            OkxAdapterMgr::GetInstance().UpdateAccountInfo();
           
	    } catch (exception e) {
            std::cerr << "UpdateAccountInfo Error: " << e.what() << std::endl;
	    }
         sleep(10);
    }

}

void MonitorOperation::GetExchangeMd() {
    while (1) {
        try {
            MdMgr::GetInstance().UpdateFromRest();
        } catch(exception e) {
            std::cerr << "GetExchangeMd Error: " << e.what() << std::endl;
        }
        sleep(5);
    }
}

MonitorOperation::MonitorOperation() {
    sendAccountAlarm = false;
    storeRiskInfo = false;

    calcAccountStatsInterval = 10; // s
    accountAlarmInterval = 2 * 60; // s
    pubAccountInfoInterval = 10;
    storeRiskInfoInterval = 5 * 60; // s
    systemMonitorInterval = 60; // s

    startTime = crypto::getCurrentTime();
    lastCalcAccountStatsTime = crypto::getCurrentTime();
    lastAccountAlarmTime = crypto::getCurrentTime();
    lastPubAccountInfoTime = crypto::getCurrentTime();
    lastStoreRiskInfoTime = crypto::getCurrentTime();
    lastSystemMonitorTime = crypto::getCurrentTime();
}

MonitorOperation::~MonitorOperation() {
    
}

bool MonitorOperation::preStart() {
    std::string host = MonitorConfig::GetInstance().GetDbAddr();
    int port = MonitorConfig::GetInstance().GetDbPort();
    std::string password = MonitorConfig::GetInstance().GetDbPassword();

    smc = new sm::SecurityManager(host.c_str(), port, password.c_str(), true);

    BinanceAdapterMgr::GetInstance().Init(smc);
    GateioAdapterMgr::GetInstance().Init(smc);
    BybitAdapterMgr::GetInstance().Init(smc);
    OkxAdapterMgr::GetInstance().Init(smc);
    AccountMgr::GetInstance().Init(smc);
    return true;
}

void MonitorOperation:: run() {
    std::thread tExchangeMd(&MonitorOperation::GetExchangeMd, this);
    tExchangeMd.detach();

    std::thread tUpdateAccountInfo(&MonitorOperation::UpdateAccountInfo, this);
    tUpdateAccountInfo.detach();

    std::thread tAccountMonitor(&MonitorOperation::RunAccountMonitor, this);
    tAccountMonitor.detach();
}