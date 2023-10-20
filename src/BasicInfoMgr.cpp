#include "BasicInfoMgr.h"

BasicInfoMgr::BasicInfoMgr() {
    ConnectRedis();
    maintainFlag = true;
    maintainRedisConnected = new thread(&BasicInfoMgr::MaintainRedisConnected, this);
}

BasicInfoMgr::~BasicInfoMgr() {
    client.disconnect();
    maintainFlag = false;
    if (maintainRedisConnected) {
        delete maintainRedisConnected;
        maintainRedisConnected = nullptr;
    }
}

BasicInfoMgr& BasicInfoMgr::GetInstance() {
	static BasicInfoMgr basicInfoMgr;
	return basicInfoMgr;
}

void BasicInfoMgr::ConnectRedis() {
    string dbAddr = MonitorConfig::GetInstance().GetDbAddr();
    int dbPort = MonitorConfig::GetInstance().GetDbPort();
    string dbPassword = MonitorConfig::GetInstance().GetDbPassword();

    client.disconnect();
    client.connect(dbAddr, dbPort, [&](const string &host, size_t port, cpp_redis::connect_state status) {
        if (status == cpp_redis::connect_state::ok) {
            isConnected = true;
            LOG_INFO("redis client connected with %s:%lu", host.c_str(), port);
            if (dbPassword.length() > 0) {
                client.auth(dbPassword, [this](const cpp_redis::reply& reply) {
                    LOG_INFO("auth info: %s", reply.as_string().c_str());
                });
            }
        }
        else if (status == cpp_redis::connect_state::dropped) {
            isConnected = false;
            LOG_ERROR("redis client disconnected with %s:%lu", host.c_str(), port);
        }
    });

    client.sync_commit();
}

void BasicInfoMgr::MaintainRedisConnected() {
    int cnt = 0;
    while (maintainFlag) {
        try {
            if(isConnected == false) {
                ConnectRedis();
            }

	    if (cnt >= 180) {
                UpdateInstrumentInfo();
		cnt = 0;
            }
        } catch(exception& e) {
            LOG_ERROR("%s", e.what());
        }
	cnt++;
        sleep(10);
    }
}

void BasicInfoMgr::Init() {
    UpdateInstrumentInfo();
}

InstrumentInfo& BasicInfoMgr::GetBasicInfo(string key) { 
    return mInstrumentInfo[key];
}

string& BasicInfoMgr::GetSysIdByOriginId(string instrumentKey) { 
    return mOriginIdSysId[instrumentKey];
}

void BasicInfoMgr::UpdateInstrumentInfo() {
    string value = "";
    GetAllInstrumentInfo(value);
    if (value.length() > 0) {
        const web::json::value& content = web::json::value::parse(value.c_str());
        if (content.is_array()) {
            auto& arr = content.as_array();
            for (auto& a: arr) {
                if (a.is_object()) {
                    InstrumentInfo info;
                    if (a.has_field("exchId")) {
                        info.exchangeType = a.at("exchId").as_string();
                    }
                    if (a.has_field("instType")) {
                        info.instrumentType = a.at("instType").as_string();
                    }
                    if (a.has_field("instId")) {
                        info.instrumentId = a.at("instId").as_string();
                    }
                    if (a.has_field("originInstId")) {
                        info.originInstrumentId = a.at("originInstId").as_string();
                    }

                    string base = "";
                    if (a.has_field("base")) {
                        base = a.at("base").as_string();
                    }

	                string quote = "";
                    if (a.has_field("quote")) {
                        quote = a.at("quote").as_string();
                    }

                    if (a.has_field("multipleVolume")) {
                        info.multipleVolume = a.at("multipleVolume").as_double();
                    }

                    string key = info.exchangeType + "|" + info.instrumentId + "|" + info.instrumentType;
                    string instrumentKey = info.exchangeType + "|" + info.originInstrumentId + "|" + info.instrumentType;

                    if (info.instrumentType == "SPOT") {
                        info.multiple = 1;
                        info.calculateType = 0;
                        info.instLeft = base;
                        info.instRight = quote;
                        info.left = base;
                        info.right = quote;
                    } else if (info.instrumentType == "FUTURES") {
                        if (a.has_field("value")) {
                            info.multiple = a.at("value").as_double();
                        }

                        string margin = "";
                        if (a.has_field("margin")) {
                            margin = a.at("margin").as_string();
                        }
                        
                        if (margin == quote) {
                            info.calculateType = 0;   // u本位
                        } else if (margin == base) {
                            info.calculateType = 1;   // 币本位
                        }
                        info.instLeft = base;
                        if (quote == "USD") {
                            info.instRight = "USDT";
                        } else {
                            info.instRight = quote;
                        }
                        info.margin = margin;
                        info.left = margin;
                        info.right = margin;

                        instrumentKey = info.exchangeType + "|" + info.originInstrumentId + "|FUTURES";
                    } else if (info.instrumentType == "SWAP") {
                        if (a.has_field("value")) {
                            info.multiple = a.at("value").as_double();
                        }
                        
                        string margin = "";
                        if (a.has_field("margin")) {
                            margin = a.at("margin").as_string();
                        }
                        
                        if (margin == quote) {
                            info.calculateType = 0;
                        } else if (margin == base) {
                            info.calculateType = 1;
                        }
                        info.instLeft = base;
                        if (quote == "USD") {
                            info.instRight = "USDT";
                        } else {
                            info.instRight = quote;
                        }
                        info.margin = margin;
                        info.left = margin;
                        info.right = margin;

                        instrumentKey = info.exchangeType + "|" + info.originInstrumentId + "|FUTURES";
                    }
                           
                    mOriginIdSysId[instrumentKey] = key;
                    mInstrumentInfo[key] = info;
                }    
            }
        }
    }
}

bool BasicInfoMgr::Get(const char* key, string& value) {
    try {
        if (isConnected) {
            client.get(key, [&value](cpp_redis::reply& reply) {
                if (reply.is_string()) {
                    value = reply.as_string();
                    return true;
                }
                else {
                    return false;
                }
            });
            client.sync_commit();
            return true;
        }
        return false;
    }
    catch (exception &e){
        LOG_ERROR("%s", e.what());
        return false;
    }
}

bool BasicInfoMgr::GetInstrumentInfo(string exchange, string instType, string instId, string& value) {
    string key = exchange + "." + instType + "." + "INSTRUMENT_INFO" + "." + instId;
    return Get(key.c_str(), value);
}

bool BasicInfoMgr::GetAllInstrumentInfo(string& value) {
    string key = "SMC.ALL.INSTRUMENTINFO";
    return Get(key.c_str(), value);
}
