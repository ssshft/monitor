#include "MsgPub.h"
#include "MonitorConfig.h"

MsgPub::MsgPub() {
	ConnectRedis();
    maintainFlag = true;
    maintainRedisConnected = new std::thread(&MsgPub::MaintainRedisConnected, this);
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
    std::string pubAddr = MonitorConfig::GetInstance().GetPubAddr();
    int pubPort = MonitorConfig::GetInstance().GetPubPort();
    std::string pubPassword = MonitorConfig::GetInstance().GetPubPassword();

    client.disconnect();
    client.connect(pubAddr, pubPort, [&](const std::string &host, size_t port, cpp_redis::connect_state status) {
        if (status == cpp_redis::connect_state::ok) {
            isConnected = true;
            LOG_INFO("redis client connected with host:{} port:{}", host, port);
            if (pubPassword.length() > 0) {
                client.auth(pubPassword, [this](const cpp_redis::reply& reply) {
                    LOG_INFO("auth info: {}", reply.as_string());
                });
            }
        }
        else if (status == cpp_redis::connect_state::dropped) {
            isConnected = false;
            LOG_ERROR("redis client disconnected with host:{} port:{}", host, port);
        }
    });

    client.sync_commit();
}

void MsgPub::MaintainRedisConnected() {
    while (maintainFlag) {
        try {
            if (isConnected == false) {
                ConnectRedis();
            }
        } catch(exception& e) {
            LOG_ERROR("{}", e.what());
        }
        sleep(10);
    }
}

void MsgPub::Publish(const std::string& channel, const std::string& msg) {
    try {
        if (isConnected) {
            client.publish(channel.c_str(), msg.c_str());
            client.sync_commit(std::chrono::seconds(5));
        }
    } catch (cpp_redis::redis_error e) {
        LOG_ERROR("publish error: {}", e.what());
    }   
}