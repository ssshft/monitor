#include "MonitorConfig.h"
#include "BasicInfoMgr.h"
#include "MdMonitor.h"
#include "AccountMonitor.h"
#include "OrderMonitor.h"
#include "BinanceAdapterMgr.h"
#include "CoinbaseAdapterMgr.h"
#include "MsgPub.h"
#include "BinanceAccountMgr.h"
#include "LarkRebot.h"
#include "taskscheduler/Scheduler.h"
#include "program_util.h"
#include "CoinbaseMdMgr.h"
#include "GateioAdapterMgr.h"
#include "BybitAdapterMgr.h"
#include "OkxAdapterMgr.h"
#include "BinanceMdMgr.h"
#include "Sqlite.h"
#include "ProductMgr.h"


std::mutex mut;
std::mutex mutRiskParameter;
std::atomic<bool> sendAccountAlarm = false;
std::atomic<bool> storeRiskInfo = false;
int sendAccountInternal = 0;
unordered_map<string, int64_t> mAccountAlarmSendTime;

void StartSetFlag() {
    sendAccountAlarm = true;
    storeRiskInfo = true;
}

void SendSystemTime() {
    try {
        string currentTimeStr = CovertToUtcStr(GetCurrentTimeUs(), false);  
        MsgCard msgCard;
        msgCard.templateId = 1;
        msgCard.title = "Systemtime";
        msgCard.object = "账户：风控系统";
        msgCard.datetime = currentTimeStr;
        msgCard.content = "风控系统运行中···";

        string systemUrl = MonitorConfig::GetInstance().GetSystemLarkUrl();
        LarkRebot::GetInstance().SendLarkMsg(msgCard, systemUrl);
        /*
        string instPath = "/opt/version/lawson/Monitor/version/inst.txt";
        fstream f;
        f.open(instPath.c_str(), ios::out);
        vector<string> v = BinanceMdMgr::GetInstance().GetCurrentMd();
        for (size_t i = 0; i < v.size(); ++i) {
            f << v[i] << "\n";
        }
        f.close();

        string currentTimeStr = CovertToUtcStr(GetCurrentTimeUs(), false);  
        MsgCard msgCard;
        msgCard.templateId = 1;
        msgCard.title = "Systemtime";
        msgCard.object = "账户：风控系统";
        msgCard.datetime = currentTimeStr;
        msgCard.content = "风控系统运行中···当前监控的品种位于" + instPath;

        unordered_map<string, ReceiveGroupInfo>& mReceiveGroupInfo = MonitorConfig::GetInstance().GetReceiveGroupInfo();
        auto iter = mReceiveGroupInfo.find("SYSTEM");
        if (iter != mReceiveGroupInfo.end()) {
            ReceiveGroupInfo receiveGroupInfo = iter->second;
            LarkRebot::GetInstance().SendGroupMsgCard(msgCard, receiveGroupInfo);
        }
        */
    } catch(exception e) {
        std::cerr << "SendSystemTime Error: " << e.what() << std::endl;
    } catch (...) {
		std::cerr << "SendSystemTime caught a unknown exception!" << std::endl;
	} 
}

void PubMsg() {
    while (1) {
    	string physicalPubChannel = MonitorConfig::GetInstance().GetPhysicalPubChannel();
    	string strategyPubChannel = MonitorConfig::GetInstance().GetStrategyPubChannel();
    	string overviewPubChannel = MonitorConfig::GetInstance().GetOverviewPubChannel();
    	string mdPubChannel = MonitorConfig::GetInstance().GetMdPubChannel();
        string mdStatusPubChannel = MonitorConfig::GetInstance().GetMdStatusPubChannel();
    
        /*
	    LOG_INFO("MdMonitor  GetCurrentStatus before");
    	unordered_map<string, string>& mMdStatus = MdMonitor::GetInstance().GetCurrentStatus();
	    LOG_INFO("MdMonitor  GetCurrentStatus end");
        auto it = mMdStatus.find("md");
    	if (it != mMdStatus.end() && it->second.length() > 0) {
        	MsgPub::GetInstance().Publish(mdPubChannel, it->second);
    	}
        */
 
	    LOG_INFO("AccountMonitor  GetCurrentStatus before");
    	unordered_map<string, string>& m = AccountMonitor::GetInstance().GetCurrentStatus();
	    LOG_INFO("AccountMonitor  GetCurrentStatus end");
        /*
    	auto iter = m.find("physical");
    	if (iter != m.end() && iter->second.length() > 0) {    
        	MsgPub::GetInstance().Publish(physicalPubChannel, iter->second);
    	}
    	iter = m.find("strategy");
    	if (iter != m.end() && iter->second.length() > 0) {
        	MsgPub::GetInstance().Publish(strategyPubChannel, iter->second);
    	}
    	iter = m.find("overview");
    	if (iter != m.end() && iter->second.length() > 0) {
        	MsgPub::GetInstance().Publish(overviewPubChannel, iter->second);            
    	}

        iter = m.find("mdstatus");
        if (iter != m.end() && iter->second.length() > 0) {
            MsgPub::GetInstance().Publish(mdStatusPubChannel, iter->second);  
        }
        */
    	sleep(10);
    }
}

void SendAccountAlarmMsg() {
    try {
        if (sendAccountAlarm) {
	        LOG_INFO("start to send account alarm msg!");
            int64_t currentTime = GetCurrentTimeUs();
            int64_t oneMinute = 60 * 1000 * 1000;
            string accountUrl = MonitorConfig::GetInstance().GetAccountLarkUrl();
            string exchangeUrl = MonitorConfig::GetInstance().GetExchangeLarkUrl();
            vector<MsgCard> vAlarmMsg = AccountMonitor::GetInstance().GetAlarmMsg();
            for (size_t j = 0; j < vAlarmMsg.size(); ++j) {
                MsgCard msg = vAlarmMsg[j];
                if (msg.title != "QueryAccount") {
                    if (msg.title == "TWAP-ADL") {
                        LarkRebot::GetInstance().SendLarkMsg(msg, accountUrl);
                    } else {
                        // need alarm twice will send alarm msg
                        string key = "";
                        if (msg.name.length() > 0) {
                            key = msg.name + "_" + msg.title;
                        } else {
                            key = to_string(msg.accountId) + "_" + msg.title;
                        }

                        auto it = mAccountAlarmSendTime.find(key);
                        if (it != mAccountAlarmSendTime.end()) {
                            if (currentTime - it->second > 6 * oneMinute) {
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


	        // int64_t currentTime = GetCurrentTimeUs();
            // int64_t oneMinute = 60 * 1000 * 1000;
            unordered_map<int, AccountVoiceCall>& mAccountVoiceCall = MonitorConfig::GetInstance().GetAccountVoiceCall();
            unordered_map<string, AccountVoiceCall>& mProductVoiceCall = MonitorConfig::GetInstance().GetProductVoiceCall();
            unordered_map<string, ReceiveGroupInfo>& mReceiveGroupInfo = MonitorConfig::GetInstance().GetReceiveGroupInfo();
            // vector<MsgCard> vAlarmMsg = AccountMonitor::GetInstance().GetAlarmMsg();
            auto iter = mReceiveGroupInfo.find("ACCOUNT");
            if (iter != mReceiveGroupInfo.end()) {
                ReceiveGroupInfo receiveGroupInfo = iter->second;
                for (size_t j = 0; j < vAlarmMsg.size(); ++j) {
                    MsgCard msg = vAlarmMsg[j];
                    if (msg.title != "QueryAccount") {
			            // need alarm twice will send alarm msg
                        string key = "";
			            if (msg.name.length() > 0) {
                            key = msg.name + "_" + msg.title;
			            } else {
                            key = to_string(msg.accountId) + "_" + msg.title;
                        }

                        auto it = mAccountAlarmSendTime.find(key);
			            if (it != mAccountAlarmSendTime.end()) {
			   	            if (currentTime - it->second > 6 * oneMinute) {
				                mAccountAlarmSendTime[key] = currentTime;
				                continue;
				            } else {
				                mAccountAlarmSendTime[key] = currentTime;
				            } 
			            } else {
				            mAccountAlarmSendTime[key] = currentTime;
			    	        continue;
			            }
			
                        //LarkRebot::GetInstance().SendGroupMsgCard(msg, receiveGroupInfo);

                        // voice call
                        if (msg.title == "Leverage" && msg.templateId == 3) {
                            auto it = mAccountVoiceCall.find(msg.accountId);
                            if (it != mAccountVoiceCall.end()) {
                                LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, it->second.groupId, it->second.vUserId);
                            }

                            auto iter = mProductVoiceCall.find(msg.name);
                            if (iter != mProductVoiceCall.end()) {
                                LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, iter->second.groupId, iter->second.vUserId);
                            }
                        }

                        if (msg.title == "RiskExposure" && msg.templateId == 3) {
                            auto it = mAccountVoiceCall.find(msg.accountId);
                            if (it != mAccountVoiceCall.end()) {
                                LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, it->second.groupId, it->second.vUserId);
                            }

                            auto iter = mProductVoiceCall.find(msg.name);
                            if (iter != mProductVoiceCall.end()) {
                                LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, iter->second.groupId, iter->second.vUserId);
                            }
                        }

			            if (msg.title == "UnderwayOrderValueThreshold" && msg.templateId == 3) {
                            auto it = mAccountVoiceCall.find(msg.accountId);
                            if (it != mAccountVoiceCall.end()) {
                                LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, it->second.groupId, it->second.vUserId);
                            }

                            auto iter = mProductVoiceCall.find(msg.name);
                            if (iter != mProductVoiceCall.end()) {
                                LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, iter->second.groupId, iter->second.vUserId);
                            }
			            }

                        if (msg.title == "NetValue" && msg.templateId == 3) {
                            auto it = mAccountVoiceCall.find(msg.accountId);
                            if (it != mAccountVoiceCall.end()) {
                                LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, it->second.groupId, it->second.vUserIdNetValue);
                            }

                            auto iter = mProductVoiceCall.find(msg.name);
                            if (iter != mProductVoiceCall.end()) {
                                LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, iter->second.groupId, iter->second.vUserIdNetValue);
                            }
                        }

                        if (msg.title == "LiquidationPrice" && msg.templateId == 3) {
                            auto it = mAccountVoiceCall.find(msg.accountId);
                            if (it != mAccountVoiceCall.end()) {
                                LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, it->second.groupId, it->second.vUserIdLiquidationPrice);
                            }
                        }
                    }
                }
            }
            /*
            auto it = mReceiveGroupInfo.find("EXCHANGE");
            if (it != mReceiveGroupInfo.end()) {
                ReceiveGroupInfo receiveGroupInfo = it->second;
                for (size_t j = 0; j < vAlarmMsg.size(); ++j) {
                    MsgCard msg = vAlarmMsg[j];
                    if (msg.title == "QueryAccount") {
                        LarkRebot::GetInstance().SendGroupMsgCard(msg, receiveGroupInfo);
                    }
                }
            }
            */
        }
    } catch(exception e) {
        std::cerr << "SendAccountAlarmMsg Error: " << e.what() << std::endl;
    } catch (...) {
		std::cerr << "SendAccountAlarmMsg caught a unknown exception!" << std::endl;
	}  
}

void SendMdAlarmMsg() {
    try {
        unordered_map<string, ReceiveGroupInfo>& mReceiveGroupInfo = MonitorConfig::GetInstance().GetReceiveGroupInfo();
        vector<MsgCard> vAlarmMsg = MdMonitor::GetInstance().GetAlarmMsg();
        auto iter = mReceiveGroupInfo.find("MARKET");
        if (iter != mReceiveGroupInfo.end()) {
            ReceiveGroupInfo receiveGroupInfo = iter->second;
            for (size_t j = 0; j < vAlarmMsg.size(); ++j) {
                MsgCard msg = vAlarmMsg[j];
                LarkRebot::GetInstance().SendGroupMsgCard(msg, receiveGroupInfo);
            }
        }

        // md price voice call
        vector<MdPriceVoiceCall>& vMdPriceVoiceCall = MonitorConfig::GetInstance().GetMdPriceVoiceCall();
        vector<MsgCard> vMdPriceMsg = BinanceMdMgr::GetInstance().GetMdPriceVoiceCallMsg();

        string mdVoiceCallGroupId = MonitorConfig::GetInstance().GetMdVoiceCallGroupId();
        vector<MsgCard> vExchangeDisconnectMsg = BinanceMdMgr::GetInstance().GetExchangeDisconnectVoiceCallMsg();
        
        if (iter != mReceiveGroupInfo.end()) {
            ReceiveGroupInfo receiveGroupInfo = iter->second;
            for (size_t j = 0; j < vMdPriceMsg.size(); ++j) {
                MsgCard msg = vMdPriceMsg[j];
                LarkRebot::GetInstance().SendGroupMsgCard(msg, receiveGroupInfo);
                for (size_t i = 0; i < vMdPriceVoiceCall.size(); ++i) {
                    if (msg.content.find(vMdPriceVoiceCall[i].symbol) != string::npos) {
                        vector<string> v;
                        vector<string> vUserId = vMdPriceVoiceCall[i].vUserId;
                        for (size_t k = 0; k < vUserId.size(); ++k) {
                            LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, vUserId[k], v);
                        }
                    }
                }
            }

            for (size_t j = 0; j < vExchangeDisconnectMsg.size(); ++j) {
                MsgCard msg = vExchangeDisconnectMsg[j];
                LarkRebot::GetInstance().SendGroupMsgCard(msg, receiveGroupInfo);
                vector<string> v;
                LarkRebot::GetInstance().SendGroupVoiceCall(msg.title, mdVoiceCallGroupId, v);
            }
        }
    } catch(exception e) {
        std::cerr << "SendMdAlarmMsg Error: " << e.what() << std::endl;
    } catch (...) {
		std::cerr << "SendMdAlarmMsg caught a unknown exception!" << std::endl;
	}  
}

void SendOrderAlarmMsg() {
    while (1) {
        try {
            unordered_map<string, ReceiveGroupInfo>& mReceiveGroupInfo = MonitorConfig::GetInstance().GetReceiveGroupInfo();
            vector<MsgCard> vAlarmMsg = OrderMonitor::GetInstance().GetAlarmMsg();
            vector<MsgCard> vMoneyAlarmMsg = OrderMonitor::GetInstance().GetOrderMoneyAlarmMsg();
            auto iter = mReceiveGroupInfo.find("ORDER");
            if (iter != mReceiveGroupInfo.end()) {
                ReceiveGroupInfo receiveGroupInfo = iter->second;
                for (size_t j = 0; j < vAlarmMsg.size(); ++j) {
                    MsgCard msg = vAlarmMsg[j];
                    LarkRebot::GetInstance().SendGroupMsgCard(msg, receiveGroupInfo);
                }

                for (size_t j = 0; j < vMoneyAlarmMsg.size(); ++j) {
                    MsgCard msg = vMoneyAlarmMsg[j];
                    LarkRebot::GetInstance().SendGroupMsgCard(msg, receiveGroupInfo);
                }
            }  
        } catch(exception e) {
            std::cerr << "SendOrderAlarmMsg Error: " << e.what() << std::endl;
        } catch (...) {
		    std::cerr << "SendOrderAlarmMsg caught a unknown exception!" << std::endl;
	    }
        sleep(60); 
    }
}

void GetCoinbaseMd() {
    while (1) {
        try {
            set<string> s = CoinbaseAdapterMgr::GetInstance().GetInstrumentList();
            CoinbaseMdMgr::GetInstance().AddNewProduct(s);
            CoinbaseMdMgr::GetInstance().GetMd();
        } catch(exception e) {
            std::cerr << "GetCoinbaseMd Error: " << e.what() << std::endl;
        }
        sleep(5);
    }
}

void GetExchangeMd() {
    while (1) {
        try {
            BinanceMdMgr::GetInstance().UpdateFromRest();
        } catch(exception e) {
            std::cerr << "GetExchangeMd Error: " << e.what() << std::endl;
        }
        sleep(5);
    }
}

void StoreRiskInfo() {
    if (storeRiskInfo) {
        try {
            DataBase db;
            vector<igmonitor::RiskInfo> vRiskInfo = BinanceAccountMgr::GetInstance().GetRiskInfo();
            db.InsertRiskInfo(vRiskInfo);

            vector<igmonitor::RiskInfo> vProductRiskInfo = ProductMgr::GetInstance().GetRiskInfo();
            db.InsertRiskInfo(vProductRiskInfo);
        } catch(exception e) {
            std::cerr << "StoreRiskInfo Error: " << e.what() << std::endl;
        }
    }
}

void SendMarketInfoAlarmMsg() {
    try {
        unordered_map<string, ReceiveGroupInfo>& mReceiveGroupInfo = MonitorConfig::GetInstance().GetReceiveGroupInfo();
        vector<MsgCard> vAlarmMsg = BinanceAccountMgr::GetInstance().GetMarketStatusAlarmMsg();

        auto iter = mReceiveGroupInfo.find("FUNDING");
        if (iter != mReceiveGroupInfo.end()) {
            ReceiveGroupInfo receiveGroupInfo = iter->second;
            for (size_t j = 0; j < vAlarmMsg.size(); ++j) {
                MsgCard msg = vAlarmMsg[j];
                LarkRebot::GetInstance().SendGroupMsgCard(msg, receiveGroupInfo);
            }
        } 
    } catch(exception e) {
        std::cerr << "SendMarketInfoAlarmMsg Error: " << e.what() << std::endl;
    } catch (...) {
		std::cerr << "SendMarketInfoAlarmMsg caught a unknown exception!" << std::endl;
	}
}

void SendFundingRateAlarmMsg() {
    try {
        unordered_map<string, ReceiveGroupInfo>& mReceiveGroupInfo = MonitorConfig::GetInstance().GetReceiveGroupInfo();
        vector<MsgCard> vAlarmMsg = AccountMonitor::GetInstance().GetFundingRateAlarmMsg();

        auto iter = mReceiveGroupInfo.find("FUNDING");
        if (iter != mReceiveGroupInfo.end()) {
            ReceiveGroupInfo receiveGroupInfo = iter->second;
            for (size_t j = 0; j < vAlarmMsg.size(); ++j) {
                MsgCard msg = vAlarmMsg[j];
                LarkRebot::GetInstance().SendGroupMsgCard(msg, receiveGroupInfo);
            }
        } 
    } catch(exception e) {
        std::cerr << "SendFundingRateAlarmMsg Error: " << e.what() << std::endl;
    } catch (...) {
		std::cerr << "SendFundingRateAlarmMsg caught a unknown exception!" << std::endl;
	}
}

void UpdateAccountInfo() {
    while (1) {
	    try {
	        ProductMgr::GetInstance().UpdateAccountInfo();
            BinanceAdapterMgr::GetInstance().UpdateAccountInfo();  // query account info
            CoinbaseAdapterMgr::GetInstance().UpdateAccountInfo();
            GateioAdapterMgr::GetInstance().UpdateAccountInfo();
            BybitAdapterMgr::GetInstance().UpdateAccountInfo();
            OkxAdapterMgr::GetInstance().UpdateAccountInfo();
            MdMonitor::GetInstance().Subscribe();                  // subscribe new md
            sleep(10);
	    } catch (exception e) {
            std::cerr << "UpdateAccountInfo Error: " << e.what() << std::endl;
	    }
        usleep(10);
    }

}

/*
int main(int argc, char* argv[]) {
    MonitorConfig::GetInstance().LoadConfig();
    int logLevel = MonitorConfig::GetInstance().GetLogLevel();
    InitLog(logLevel);
    BybitAdapterMgr::GetInstance().UpdateAccountInfo();
    while (1) {
        NanoLog::poll();
        sleep(1);
    }
    return 0;
}
*/

int main(int argc, char* argv[]) {
    string currentTimeStr = CovertToUtcStr(GetCurrentTimeUs(), false);
    cout << currentTimeStr << " risk_monitor server executed..." << endl;
    for(auto i = 0; i < argc; ++i){
        cout << currentTimeStr << " argv[" << i << "]:" << argv[i] << endl;
    }
    if (argc < 2) {
        cout << "Incorrect parameters, please check!" << endl;
        return 0;
    }
    ::chdir(argv[1]);

    string program = "risk_monitor";
    int currentPid = getpid();
    auto filePid = crypto::get_program_pid(program);
    if(!crypto::ensure_one_instance(program) && currentPid != filePid) {
        string errormsg = program + " with pid=" + std::to_string(filePid) + " already exists";
        std::cout << errormsg.c_str() << std::endl;
        return 0;
    }
    crypto::write_program_pid(program);

    MonitorConfig::GetInstance().LoadConfig();
    int logLevel = MonitorConfig::GetInstance().GetLogLevel();
    InitLog(logLevel);

    Bosma::Scheduler::GetInstance().cron("1 0 * * *", &InitLog, logLevel);  // 重新初始化log文件
    Bosma::Scheduler::GetInstance().in(std::chrono::minutes(10), &StartSetFlag);
    //Bosma::Scheduler::GetInstance().every(std::chrono::minutes(1), &PubMsg);
    Bosma::Scheduler::GetInstance().every(std::chrono::minutes(2), &SendAccountAlarmMsg);
    int systemTimeInterval = MonitorConfig::GetInstance().GetSystemAlarmTime();
    Bosma::Scheduler::GetInstance().every(std::chrono::minutes(systemTimeInterval), &SendSystemTime);
    //Bosma::Scheduler::GetInstance().every(std::chrono::minutes(1), &SendMdAlarmMsg);
    Bosma::Scheduler::GetInstance().every(std::chrono::minutes(5), &StoreRiskInfo);
    //Bosma::Scheduler::GetInstance().cron("30 1 * * *", &SendMarketInfoAlarmMsg);
    //Bosma::Scheduler::GetInstance().cron("0 * * * *", &SendFundingRateAlarmMsg);  // 每小时运行一次
    //thread t(SendOrderAlarmMsg);
    //thread tCoinbaseMd(GetCoinbaseMd);
    thread tPubMsg(PubMsg);
    thread tExchangeMd(GetExchangeMd);
    thread tUpdateAccountInfo(UpdateAccountInfo);
    BasicInfoMgr::GetInstance().Init();

    bool running = true;
    while (running) {
        NanoLog::poll();
        sleep(1);
    }

    return 0;
}
