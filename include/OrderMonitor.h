#pragma once

#include "Utility.h"
#include "DataBase.h"

class OrderMonitor {
public:
    ~OrderMonitor();
    static OrderMonitor& GetInstance();
    vector<MsgCard> GetAlarmMsg();
    vector<MsgCard> GetOrderMoneyAlarmMsg();

private:
    OrderMonitor();
    DataBase db;
    double orderNetValuePercent;
    double orderUsdtValueDown;
    double orderUsdtValueUp;
    OrderValueThreshold orderValueThresholdTotal;
    unordered_map<int, OrderValueThreshold> mOrderValueThreshold;
};