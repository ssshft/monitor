#include "MdMonitor.h"
#include "MonitorConfig.h"
#include "DataQueue.h"
#include "BinanceMdMgr.h"
#include "BasicInfoMgr.h"
#include "BinanceAdapterMgr.h"
#include "BinanceSystemMgr.h"
#include "GateioAdapterMgr.h"
#include "BybitAdapterMgr.h"
#include "OkxAdapterMgr.h"

MdMonitor::MdMonitor() {
    vCurrentChannel.clear();

    /*
	ConnectRedis();
    maintainFlag = true;
    dealSubFlag = true;
    f = MdMonitor::OnSubMessage;
    maintainRedisConnected = new thread(&MdMonitor::MaintainRedisConnected, this);
    dealSubData = new thread(&MdMonitor::DealSubData, this);
    */
}

MdMonitor::~MdMonitor() {
    /*
    redisSub.disconnect();
    maintainFlag = false;
    dealSubFlag = false;
    if (maintainRedisConnected) {
        delete maintainRedisConnected;
        maintainRedisConnected = nullptr;
    }
    if (dealSubData) {
        delete dealSubData;
        dealSubData = nullptr;
    }
    */
}

MdMonitor& MdMonitor::GetInstance() {
	static MdMonitor mdMonitor;
	return mdMonitor;
}

void MdMonitor::ConnectRedis() {
    string mdAddr = MonitorConfig::GetInstance().GetMdAddr();
    int mdPort = MonitorConfig::GetInstance().GetMdPort();
    string mdPassword = MonitorConfig::GetInstance().GetMdPassword();

    redisSub.disconnect();
    redisSub.connect(mdAddr, mdPort, [&](const string &host, size_t port, cpp_redis::connect_state status) {
        if (status == cpp_redis::connect_state::ok) {
            isConnected = true;
            LOG_INFO("redis client connected with %s:%lu", host.c_str(), port);
            if (mdPassword.length() > 0) {
                redisSub.auth(mdPassword, [this](const cpp_redis::reply& reply) {
                    LOG_INFO("auth info: %s", reply.as_string().c_str());
                });
            }
        }
        else if (status == cpp_redis::connect_state::dropped) {
            isConnected = false;
            LOG_ERROR("redis client disconnected with %s:%lu", host.c_str(), port);
        }
    });
    
}

void MdMonitor::Subscribe() {
    vector<string>& vMdChannels = MonitorConfig::GetInstance().GetMdChannels();      // other symbol channels
    vector<string>& vSubChannels = MonitorConfig::GetInstance().GetMdSubChannels();   // TRADES,DEPTH20,KLINE_1m,FUNDING_RATE
    set<string> sInstrumentAdapter = BinanceAdapterMgr::GetInstance().GetInstrumentList();
    //set<string> sInstrumentSystem = BinanceSystemMgr::GetInstance().GetInstrumentList();
    set<string> sInstrumentGateioAdapter = GateioAdapterMgr::GetInstance().GetInstrumentList();
    set<string> sInstrumentBybitAdapter = BybitAdapterMgr::GetInstance().GetInstrumentList();
    set<string> sInstrumentOkxAdapter = OkxAdapterMgr::GetInstance().GetInstrumentList();

    //sInstrumentAdapter.insert(sInstrumentSystem.begin(), sInstrumentSystem.end());
    sInstrumentAdapter.insert(sInstrumentGateioAdapter.begin(), sInstrumentGateioAdapter.end());
    sInstrumentAdapter.insert(sInstrumentBybitAdapter.begin(), sInstrumentBybitAdapter.end());
    sInstrumentAdapter.insert(sInstrumentOkxAdapter.begin(), sInstrumentOkxAdapter.end());

    vector<string> vInstrumentKeys;
    for (auto iter = sInstrumentAdapter.begin(); iter != sInstrumentAdapter.end(); ++iter) {
        string instKey = *iter;
        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
        if (key.length() > 0) {
            vInstrumentKeys.push_back(key);
        }
    }

    vector<string> vUpdateChannel;
    for (size_t i = 0; i < vSubChannels.size(); ++i) {
        string topic = vSubChannels[i];
        for (size_t j = 0; j < vInstrumentKeys.size(); ++j) {
            string instrumentKey = vInstrumentKeys[j];
            vector<string> v;
            SplitString(instrumentKey, "|", v);
            if (v.size() >= 3) {
                string exchangeType = v[0]; 
                string instrumentType = v[2];
                string instrumentId = v[1];

                if (exchangeType == "BYBIT") { // bybit 目前只有一档的行情
                    string channel = exchangeType + "." + instrumentType + "." + "DEPTH1" + "." + instrumentId;
                    vUpdateChannel.push_back(channel); 
                } else {
                    string channel = exchangeType + "." + instrumentType + "." + topic + "." + instrumentId;
                    vUpdateChannel.push_back(channel);
                }
            }
        }
    }
    vUpdateChannel.insert(vUpdateChannel.end(), vMdChannels.begin(), vMdChannels.end());

    sort(vCurrentChannel.begin(), vCurrentChannel.end());
    sort(vUpdateChannel.begin(), vUpdateChannel.end());

    vector<string> vSame;
    vector<string> vOldChannel;
    vector<string> vNewChannel;
    set_intersection(vCurrentChannel.begin(), vCurrentChannel.end(), vUpdateChannel.begin(), vUpdateChannel.end(), inserter(vSame, vSame.begin()));
    set_difference(vCurrentChannel.begin(), vCurrentChannel.end(), vSame.begin(), vSame.end(), inserter(vOldChannel, vOldChannel.begin()));
    set_difference(vUpdateChannel.begin(), vUpdateChannel.end(), vSame.begin(), vSame.end(), inserter(vNewChannel, vNewChannel.begin()));

    // 取消订阅
    // for (size_t i = 0; i < vOldChannel.size(); ++i) {
    //     string channel = vOldChannel[i];
    //     redisSub.unsubscribe(channel);
    // }

    /*
    for (size_t i = 0; i < vNewChannel.size(); ++i) {
        string channel = vNewChannel[i];
        redisSub.subscribe(channel, f);
    }

    if (vNewChannel.size() > 0 || vOldChannel.size() > 0) {
        redisSub.commit();
    }
    */
    vCurrentChannel = vUpdateChannel;
}

void MdMonitor::OnSubMessage(const string& channel, const string& msg) {
    SubData data;
    data.channel = channel;
	data.content = msg;
    DataQueue::GetInstance().InsertMdData(data);
}

void MdMonitor::MaintainRedisConnected() {
    while (maintainFlag) {
        try {
            if(isConnected == false) {
                vCurrentChannel.clear();
                ConnectRedis();
                Subscribe();
            }
        } catch(exception& e) {
            LOG_ERROR("%s", e.what());
        }
        sleep(10);
    }
}

void MdMonitor::DealSubData() {
    while (dealSubFlag) {
		SubData data;
		DataQueue::GetInstance().PopMdData(data);

	    try {        
            vector<string> v;
            SplitString(data.channel, ".", v);
            if (v.size() >= 3) {
                string exchangeType = v[0];
                string topic = v[2];
                BinanceMdMgr::GetInstance().OnSubMessage(topic, web::json::value::parse(data.content.c_str()));
            }
        } catch(exception& e) {
    	    LOG_DEBUG("Recv Error Message : '%s' content: %s", e.what(), data.content.c_str());
	    }
    }
}

unordered_map<string, string>& MdMonitor::GetCurrentStatus() {
    mMdStatus["md"] = BinanceMdMgr::GetInstance().Serialize();
    return mMdStatus;
}

vector<MsgCard> MdMonitor::GetAlarmMsg() {
    vector<MsgCard> vDepth = BinanceMdMgr::GetInstance().GetAlarmMsg();
    vector<MsgCard> vKline = BinanceMdMgr::GetInstance().GetKlineAlarmMsg();
    vector<MsgCard> vTrade = BinanceMdMgr::GetInstance().GetTradeAlarmMsg();
    vDepth.insert(vDepth.end(), vKline.begin(), vKline.end());
    vDepth.insert(vDepth.end(), vTrade.begin(), vTrade.end());
    return vDepth;
}

vector<string>& MdMonitor::GetCurrentChannel() {
    return vCurrentChannel;
}