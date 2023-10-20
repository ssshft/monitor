#pragma once

#include <cpprest/json.h>
#include <string>
#include <map>
#include <queue>
#include "DataArray.h"
#include "Utility.h"

using namespace std;


class BinanceMdMgr {
public:
	static BinanceMdMgr& GetInstance();
	~BinanceMdMgr();

    void OnSubMessage(string topic, const web::json::value& content);
	void UpdateFromRest();
    void UpdateDepth(const web::json::value& content);
	void UpdateKline(const web::json::value& content);
	void UpdateRate(const web::json::value& content);
	void UpdateTrade(const web::json::value& content);
	Depth GetLastDepth(string key);
	double GetMidPrice(string key);
	double GetAssetPrice(string asset, string exchangeStr);  // 获取对应asset现货的价格（u计价），若没有则获取永续合约的价格（u计价）
	double GetFundingRate(string key);
	string Serialize();
	web::json::value GetFundingRateValue();
	vector<MsgCard> GetAlarmMsg();
	vector<MsgCard> GetKlineAlarmMsg();
	vector<MsgCard> GetTradeAlarmMsg();
	vector<string> GetCurrentMd();
	vector<MsgCard> GetMdPriceVoiceCallMsg();
	vector<MsgCard> GetExchangeDisconnectVoiceCallMsg();

private:
	BinanceMdMgr();

	unordered_map<string, DataArray<Depth>> mDepth;
	unordered_map<string, DataArray<Kline>> mKline;
	unordered_map<string, DataArray<Rate>> mRate;
	unordered_map<string, DataArray<Trade>> mTrade;

	int depthLen;
	int klineLen;
	int rateLen;
	int tradeLen;

	double delayAlarmTime;
	double delayKlineAlarmTime;
	double delayTradeAlarmTime;
	vector<MdPriceVoiceCall> vMdPriceVoiceCall;
};
