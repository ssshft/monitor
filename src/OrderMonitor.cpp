#include "OrderMonitor.h"
#include "MonitorConfig.h"
#include "BasicInfoMgr.h"
#include "BinanceMdMgr.h"
#include "BinanceAccountMgr.h"


OrderMonitor::OrderMonitor() {
    orderNetValuePercent = MonitorConfig::GetInstance().GetOrderNetValuePercent();
    orderUsdtValueDown = MonitorConfig::GetInstance().GetOrderUsdtValueDown();
    orderUsdtValueUp = MonitorConfig::GetInstance().GetOrderUsdtValueUp();

    orderValueThresholdTotal = MonitorConfig::GetInstance().GetOrderValueThresholdTotal();
    mOrderValueThreshold = MonitorConfig::GetInstance().GetOrderValueThreshold();
}

OrderMonitor::~OrderMonitor() {
}

OrderMonitor& OrderMonitor::GetInstance() {
	static OrderMonitor orderMonitor;
	return orderMonitor;
}

vector<MsgCard> OrderMonitor::GetAlarmMsg() {
    vector<MsgCard> v;
    //vector<DbOrder> vInSufficient = db.QueryOrder(9, "insufficient");  // OST_REJECTED  资金不足
    //vector<DbOrder> vLimit = db.QueryOrder(9, "limit");     // OST_REJECTED  下单限频

    vector<DbOrder> vRejected = db.QueryOrder(9);     // OST_REJECTED
    vector<DbOrder> vPending = db.QueryOrder(1);       // OST_PENDING
    vector<DbOrder> vCanceling = db.QueryOrder(4);     // OST_CANCELING

    string currentTimeStr = CovertToUtcStr(GetCurrentTimeUs(), false);
    unordered_map<string, string> mInSufficientContent;
    unordered_map<string, string> mLimitContent;
    for (size_t i = 0; i < vRejected.size(); ++i) {
        DbOrder order = vRejected[i];
        if (order.errMsg.find("insufficient") != string::npos) {
            stringstream ss;
            ss << "【" << order.clientOrderId << "】号订单因【" << order.errMsg << "】原因被【" << OrderStatusEnum2StrMap[OrderStatus(order.status)] << "】，订单信息【" << ExchangeTypeEnum2StrMap[ExchangeType(order.exchangeId)] << "】【" << order.side << "】【" << order.quantity << "】个【" << order.symbol << "】价格【" << order.price << "】"; 
            string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(order.accountId);
            mInSufficientContent[accountName] += ss.str();
            /*
            MsgCard msgCard;
            msgCard.templateId = 2;
            msgCard.title = "资金不足";
            msgCard.object = "账户：" + accountName;
            msgCard.datetime = currentTimeStr;
            msgCard.content = ss.str();
            v.emplace_back(msgCard);
            */
        } else if (order.errMsg.find("limit") != string::npos) {
            stringstream ss;
            ss << "【" << order.clientOrderId << "】号订单因【" << order.errMsg << "】原因被【" << OrderStatusEnum2StrMap[OrderStatus(order.status)] << "】，订单信息【" << ExchangeTypeEnum2StrMap[ExchangeType(order.exchangeId)] << "】【" << order.side << "】【" << order.quantity << "】个【" << order.symbol << "】价格【" << order.price << "】"; 
            string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(order.accountId);
            mLimitContent[accountName] += ss.str();
            /*
            MsgCard msgCard;
            msgCard.templateId = 2;
            msgCard.title = "下单限频";
            msgCard.object = "账户：" + accountName;
            msgCard.datetime = currentTimeStr;
            msgCard.content = ss.str();
            v.emplace_back(msgCard);
            */
        } else {
            /*
            stringstream ss;
            ss << "【" << order.clientOrderId << "】号订单因【" << order.errMsg << "】原因被【" << OrderStatusEnum2StrMap[OrderStatus(order.status)] << "】，订单信息【" << ExchangeTypeEnum2StrMap[ExchangeType(order.exchangeId)] << "】【" << order.side << "】【" << order.quantity << "】个【" << order.symbol << "】价格【" << order.price << "】"; 
            string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(order.accountId);
            MsgCard msgCard;
            msgCard.templateId = 2;
            msgCard.title = "报单拒单";
            msgCard.object = "账户：" + accountName;
            msgCard.datetime = currentTimeStr;
            msgCard.content = ss.str();
            v.emplace_back(msgCard);
            */
        }
    }

    for (auto iter = mInSufficientContent.begin(); iter != mInSufficientContent.end(); ++iter) {
        if (iter->second.length() > 0) {
            MsgCard msgCard;
            msgCard.templateId = 2;
            msgCard.title = "资金不足";
            msgCard.object = "账户：" + iter->first;
            msgCard.datetime = currentTimeStr;
            msgCard.content = iter->second;
            v.emplace_back(msgCard);
        }
    }

    for (auto iter = mLimitContent.begin(); iter != mLimitContent.end(); ++iter) {
        if (iter->second.length() > 0) {
            MsgCard msgCard;
            msgCard.templateId = 2;
            msgCard.title = "下单限频";
            msgCard.object = "账户：" + iter->first;
            msgCard.datetime = currentTimeStr;
            msgCard.content = iter->second;
            v.emplace_back(msgCard);
        }
    }
    
/*
    for (size_t i = 0; i < vInSufficient.size(); ++i) {
        DbOrder order = vInSufficient[i];
        stringstream ss;
        ss << "【" << order.clientOrderId << "】号订单因【" << order.errMsg << "】原因被【" << OrderStatusEnum2StrMap[OrderStatus(order.status)] << "】，订单信息【" << ExchangeTypeEnum2StrMap[ExchangeType(order.exchangeId)] << "】【" << order.side << "】【" << order.quantity << "】个【" << order.symbol << "】价格【" << order.price << "】"; 
        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(order.accountId);
        MsgCard msgCard;
        msgCard.templateId = 2;
        msgCard.title = "资金不足";
        msgCard.object = "账户：" + accountName;
        msgCard.datetime = currentTimeStr;
        msgCard.content = ss.str();
        v.emplace_back(msgCard);
    }

    for (size_t i = 0; i < vLimit.size(); ++i) {
        DbOrder order = vLimit[i];
        stringstream ss;
        ss << "【" << order.clientOrderId << "】号订单因【" << order.errMsg << "】原因被【" << OrderStatusEnum2StrMap[OrderStatus(order.status)] << "】，订单信息【" << ExchangeTypeEnum2StrMap[ExchangeType(order.exchangeId)] << "】【" << order.side << "】【" << order.quantity << "】个【" << order.symbol << "】价格【" << order.price << "】"; 
        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(order.accountId);
        MsgCard msgCard;
        msgCard.templateId = 2;
        msgCard.title = "下单限频";
        msgCard.object = "账户：" + accountName;
        msgCard.datetime = currentTimeStr;
        msgCard.content = ss.str();
        v.emplace_back(msgCard);
    }
*/
    for (size_t i = 0; i < vPending.size(); ++i) {
        DbOrder order = vPending[i];
        stringstream ss;
        ss << "【" << order.clientOrderId << "】号订单因【" << order.errMsg << "】原因被【" << OrderStatusEnum2StrMap[OrderStatus(order.status)] << "】，订单信息【" << ExchangeTypeEnum2StrMap[ExchangeType(order.exchangeId)] << "】【" << order.side << "】【" << order.quantity << "】个【" << order.symbol << "】价格【" << order.price << "】"; 
        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(order.accountId);
        MsgCard msgCard;
        msgCard.templateId = 2;
        msgCard.title = "长时间未更新状态";
        msgCard.object = "账户：" + accountName;
        msgCard.datetime = currentTimeStr;
        msgCard.content = ss.str();
        v.emplace_back(msgCard);
    }

    for (size_t i = 0; i < vCanceling.size(); ++i) {
        DbOrder order = vCanceling[i];
        stringstream ss;
        ss << "【" << order.clientOrderId << "】号订单因【" << order.errMsg << "】原因被【" << OrderStatusEnum2StrMap[OrderStatus(order.status)] << "】，订单信息【" << ExchangeTypeEnum2StrMap[ExchangeType(order.exchangeId)] << "】【" << order.side << "】【" << order.quantity << "】个【" << order.symbol << "】价格【" << order.price << "】"; 
        string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(order.accountId);
        MsgCard msgCard;
        msgCard.templateId = 2;
        msgCard.title = "长时间未更新状态";
        msgCard.object = "账户：" + accountName;
        msgCard.datetime = currentTimeStr;
        msgCard.content = ss.str();
        v.emplace_back(msgCard);
    }
    
    return v;
}

vector<MsgCard> OrderMonitor::GetOrderMoneyAlarmMsg() {
    vector<MsgCard> v;

	int64_t currentUs = GetCurrentTimeUs();
	int64_t oneMinute = 60 * 1000 * 1000;
	int64_t beforeCurrentOneMinute = currentUs - oneMinute;
	string startTime = CovertToUtcStr(beforeCurrentOneMinute, false);
	string endTime = CovertToUtcStr(currentUs, false);

    vector<DbOrder> vOrder = db.QueryOrder(startTime, endTime);
    for (size_t i = 0; i < vOrder.size(); ++i) {
        DbOrder order = vOrder[i];
        int accountId = order.accountId;
        string symbol = order.symbol;
        double quantity = order.quantity;
        double price = order.price;
        string exchangeStr = ExchangeTypeEnum2StrMap[ExchangeType(vOrder[i].exchangeId)];
        string instKey = "";
        AccountType accountType =  (AccountType)order.accountType;
        if (accountType == AT_SPOT) {
            instKey = exchangeStr + "|" + symbol + "|SPOT";
        } else if (accountType == AT_CFUTURES || accountType == AT_UFUTURES) {
            instKey = exchangeStr + "|" + symbol + "|FUTURES";
        }

        string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instKey);
        InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
        string base = info.instLeft;
        string quote = info.instRight;

        double money = 0.0;
        if (accountType == AT_SPOT) {
            if (quote == "USDT") {
                money = quantity * price;
            } else {
                double priceBase = BinanceMdMgr::GetInstance().GetAssetPrice(base, exchangeStr);
                money = quantity * priceBase;
            }
        } else if (accountType == AT_UFUTURES) {
            money = quantity * price * info.multiple;
        } else if (accountType == AT_CFUTURES) {
            money = quantity * info.multiple ;
        } 

        double netValue = 0.0;
        int customerId = 0;
        BinanceAccountItem* item = BinanceAccountMgr::GetInstance().GetAccountItem(accountId);
        if (item) {
            netValue = item->GetRiskInfo().netValueD;  // convet to usdt value
            string baseAsset = item->GetRiskInfo().baseAsset;
            if (baseAsset != "USDT") {
                netValue = netValue * BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
            }

            customerId = item->GetAccountId();
        }

        OrderValueThreshold orderValueThreshold = orderValueThresholdTotal;
        auto iter = mOrderValueThreshold.find(customerId);
        if (iter != mOrderValueThreshold.end()) {
            orderValueThreshold = iter->second;
        }

        double alarmValue = 0;
        if (orderValueThreshold.orderNetValuePercent > 0 && orderValueThreshold.orderUsdtValueDown > 0 && orderValueThreshold.orderUsdtValueUp > 0) {
            alarmValue = min(max(orderValueThreshold.orderUsdtValueDown, netValue * orderValueThreshold.orderNetValuePercent), orderValueThreshold.orderUsdtValueUp);
        } else if (orderValueThreshold.orderUsdtValueUp > 0) {
            alarmValue = orderValueThreshold.orderUsdtValueUp;
        }
        
        if (money >= alarmValue) {
            stringstream ss;
            ss << "【" << order.clientOrderId << "】号订单报单金额超限" << "，订单信息【" << ExchangeTypeEnum2StrMap[ExchangeType(order.exchangeId)] << "】【" << order.side << "】【" <<  OrderStatusEnum2StrMap[OrderStatus(order.status)] << "】【" << order.quantity << "】个【" << order.symbol << "】价格【" << order.price << "】"; 

            string accountName = MonitorConfig::GetInstance().GetAccountNameByAccountId(accountId);
            MsgCard msgCard;
            msgCard.templateId = 2;
            msgCard.title = "报单金额超限";
            msgCard.object = "账户：" + accountName;
            msgCard.datetime = startTime;
            msgCard.content = ss.str();
            v.emplace_back(msgCard);
        }
    }

    return v;
}