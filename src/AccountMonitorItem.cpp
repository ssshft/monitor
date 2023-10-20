#include "AccountMonitorItem.h"
#include "BinanceAccountItem.h"
#include "BinanceAccountMgr.h"
#include "DataQueue.h"


AccountMonitorItem::AccountMonitorItem(AccountMonitorInfo info) {
    accountMonitorInfo = info;
    ConnectRedis();
    SubScirbe();

    maintainFlag = true;
    maintainRedisConnected = new thread(&AccountMonitorItem::MaintainRedisConnected, this);
}

AccountMonitorItem::~AccountMonitorItem() {
    maintainFlag = false;
    if (maintainRedisConnected) {
        delete maintainRedisConnected;
        maintainRedisConnected = nullptr;
    }
}

void AccountMonitorItem::ConnectRedis() {
    string tradeAddr = accountMonitorInfo.addr;
    int tradePort = accountMonitorInfo.port;
    string password = accountMonitorInfo.password;

    redisSub.disconnect();
    redisSub.connect(tradeAddr, tradePort, [&](const string &host, size_t port, cpp_redis::connect_state status) {
        if (status == cpp_redis::connect_state::ok) {
            isConnected = true;
            LOG_INFO("redis client connected with %s:%lu", host.c_str(), port);
            if (password.length() > 0) {
                redisSub.auth(password, [this](const cpp_redis::reply& reply) {
                    LOG_INFO("auth info: %s", reply.as_string().c_str());
                });
            }
        }
        else if (status == cpp_redis::connect_state::dropped) {
            isConnected = false;
            LOG_ERROR("redis client disconnected with %s:%lu", host.c_str(), port);
        }
    });

    redisSub.commit();
}

void AccountMonitorItem::SubScirbe() {
    vector<string> vSubChannels;
    SplitString(accountMonitorInfo.channels, ",", vSubChannels);

    function<void (const string& channel, const string& msg)> f;
	f = AccountMonitorItem::OnSubMessage;

	for (auto& channel: vSubChannels)
		redisSub.subscribe(channel, f);
	redisSub.commit();
}

void AccountMonitorItem::OnSubMessage(const string& channel, const string& msg) {
    SubData data;
    data.channel = channel;
	data.content = msg;
    DataQueue::GetInstance().InsertTradeData(data);
}

void AccountMonitorItem::MaintainRedisConnected() {
    while (maintainFlag) {
        try {
            if(isConnected == false) {
                ConnectRedis();
                SubScirbe();
            }
        } catch(exception& e) {
            LOG_ERROR("%s", e.what());
        }
        sleep(10);
    }
}
