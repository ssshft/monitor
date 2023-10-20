#include "MsgPub.h"
#include "MonitorConfig.h"

MsgPub::MsgPub() {
	ConnectRedis();
    //SubConnectRedis();
    //SubScirbe();
    maintainFlag = true;
    maintainRedisConnected = new thread(&MsgPub::MaintainRedisConnected, this);
}

 MsgPub::~MsgPub() {
    maintainFlag = false;
    if (maintainRedisConnected) {
        delete maintainRedisConnected;
        maintainRedisConnected = nullptr;
    }
 }

MsgPub& MsgPub::GetInstance() {
	static MsgPub msgPub;
	return msgPub;
}

void MsgPub::ConnectRedis() {
    string pubAddr = MonitorConfig::GetInstance().GetPubAddr();
    int pubPort = MonitorConfig::GetInstance().GetPubPort();
    string pubPassword = MonitorConfig::GetInstance().GetPubPassword();

    client.disconnect();
    client.connect(pubAddr, pubPort, [&](const string &host, size_t port, cpp_redis::connect_state status) {
        if (status == cpp_redis::connect_state::ok) {
            isConnected = true;
            LOG_INFO("redis client connected with %s:%lu", host.c_str(), port);
            if (pubPassword.length() > 0) {
                client.auth(pubPassword, [this](const cpp_redis::reply& reply) {
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

void MsgPub::SubConnectRedis() {
    string subAddr = MonitorConfig::GetInstance().GetPubAddr();
    int subPort = MonitorConfig::GetInstance().GetPubPort();
    string subPassword = MonitorConfig::GetInstance().GetPubPassword();

    redisSub.disconnect();
    redisSub.connect(subAddr, subPort, [&](const string &host, size_t port, cpp_redis::connect_state status) {
        if (status == cpp_redis::connect_state::ok) {
            LOG_INFO("sub connect redis client connected with %s:%lu", host.c_str(), port);
            if (subPassword.length() > 0) {
                redisSub.auth(subPassword, [this](const cpp_redis::reply& reply) {
                    LOG_INFO("sub auth info: %s", reply.as_string().c_str());
                });
            }
        }
        else if (status == cpp_redis::connect_state::dropped) {
            LOG_ERROR("sub redis client disconnected with %s:%lu", host.c_str(), port);
        }
    });

    redisSub.commit();
}

void MsgPub::MaintainRedisConnected() {
    while (maintainFlag) {
        try {
            if(isConnected == false) {
                ConnectRedis();
            }

            //if (redisSub.is_connected() == false){
            //    SubConnectRedis();
            //}
        } catch(exception& e) {
            LOG_ERROR("%s", e.what());
        }
        sleep(10);
    }
}

void MsgPub::Publish(string channel, const string& msg) {
    try {
        if (isConnected) {
            client.publish(channel.c_str(), msg.c_str());
            client.sync_commit(std::chrono::seconds(5));
        }
    } catch (cpp_redis::redis_error e) {
        std::cout << "publish error: " << e.what() << std::endl;
    }   
}

void MsgPub::SubScirbe() {
    string paraChannel = MonitorConfig::GetInstance().GetParaChannel();
    function<void (const string& channel, const string& msg)> f;
	f = MsgPub::OnSubMessage;
	redisSub.subscribe(paraChannel, f);
	redisSub.commit();
}

void MsgPub::OnSubMessage(const string& channel, const string& msg) {
/*
    if (channel == "RiskParameter") {
        std::lock_guard<std::mutex> lk(mutRiskParameter);
        const web::json::value& content = web::json::value::parse(msg.c_str());
        if (content.has_field("MdPrice")) {
            vector<MdPriceVoiceCall> v;
            web::json::value const& mdPrice = content.at("MdPrice");
            if (mdPrice.is_array()) {
                auto mdPriceArray = mdPrice.as_array();
                for (auto& md : mdPriceArray) {
                    MdPriceVoiceCall mdPriceVoiceCall;
                    if (md.has_field("symbol")) {
                        mdPriceVoiceCall.symbol = md.at("symbol").as_string();
                    }
                    if (md.has_field("price")) {
                        mdPriceVoiceCall.price = md.at("price").as_double();
                    }
                    if (md.has_field("mode")) {
                        mdPriceVoiceCall.mode = md.at("mode").as_integer();
                    }
                    v.emplace_back(mdPriceVoiceCall);
                }
            }
            MonitorConfig::GetInstance().SetMdPriceVoiceCall(v);
        }
    }

   */
}
