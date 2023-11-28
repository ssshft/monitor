#include "BinanceMdMgr.h"
#include "MonitorConfig.h"
#include "MdMonitor.h"
#include "ExchangeRestMd.h"

BinanceMdMgr::BinanceMdMgr() {
	depthLen = 10;
	klineLen = 10;
	rateLen = 10;
	tradeLen = 10;
	delayAlarmTime = MonitorConfig::GetInstance().GetMdAlarm();
	delayKlineAlarmTime = MonitorConfig::GetInstance().GetMdKlineAlarm();
	delayTradeAlarmTime = MonitorConfig::GetInstance().GetMdTradeAlarm();
	vMdPriceVoiceCall = MonitorConfig::GetInstance().GetMdPriceVoiceCall();
}

BinanceMdMgr::~BinanceMdMgr() {
	mDepth.clear();
	mKline.clear();
}

BinanceMdMgr& BinanceMdMgr::GetInstance() {
	static BinanceMdMgr binanceMdMgr;
	return binanceMdMgr;
}

void BinanceMdMgr::OnSubMessage(string topic, const web::json::value& content) {
	if (topic.find("DEPTH") != string::npos) {
		UpdateDepth(content);
	} else if (topic.find("KLINE") != string::npos) {
		UpdateKline(content);
	} else if (topic.find("FUNDING_RATE") != string::npos) {
		UpdateRate(content);
	} else if (topic.find("TRADES") != string::npos) {
		UpdateTrade(content);
	}
}

void BinanceMdMgr::UpdateFromRest() {
	vector<string>& vCurrentChannel = MdMonitor::GetInstance().GetCurrentChannel();
	set<string> s;
	for (size_t i = 0; i < vCurrentChannel.size(); ++i) {
		s.insert(vCurrentChannel[i]);
	}

	for (auto it = s.begin(); it != s.end(); ++it) {
		string channel = *it;
		vector<string> v;
        SplitString(channel, ".", v);
		string exchangeType = v[0];
		string instType = v[1];
		string instId = v[3];
		Depth depth = ExchangeRestMd::GetInstance().GetDepth(instId, instType, exchangeType);

		if (depth.ts > 0) {
			string key = depth.exchangeType + "|" +  depth.instrumentId + "|" + depth.instrumentType;
			auto iter = mDepth.find(key);
			if (iter != mDepth.end()) {
				iter->second.Add(depth);
			} else {
				DataArray<Depth> dataArr(depthLen);
				dataArr.Add(depth);
				mDepth[key] = dataArr;
			}
		}

		usleep(100000);
	}
}

void BinanceMdMgr::UpdateDepth(const web::json::value& content) {
	Depth depth;
	depth.exchangeType = content.at("exchId").as_string();
	depth.instrumentType = content.at("instType").as_string();
	depth.marketType = content.at("marketType").as_string();
	depth.instrumentId = content.at("instId").as_string();
	const web::json::array& asksList = content.at("asksList").as_array();
	for (auto& asks: asksList) {
		const web::json::array& asksArr = asks.as_array();
		double askPrice = asksArr.at(0).as_double();
		double askVolume = asksArr.at(1).as_double();
		depth.askP.push_back(askPrice);
		depth.askV.push_back(askVolume);
	}

	const web::json::array& bidsList = content.at("bidsList").as_array();
	for (auto& bids: bidsList) {
		const web::json::array& bidsArr = bids.as_array();
		double bidPrice = bidsArr.at(0).as_double();
		double bidVolume = bidsArr.at(1).as_double();
		depth.bidP.push_back(bidPrice);
		depth.bidV.push_back(bidVolume);
	}
	
	depth.ts = content.at("ts").as_number().to_int64();
	if (depth.ts <= 0) {
		depth.ts = GetCurrentTimeUs();
	}
	depth.tsNet = content.at("tsNet").as_number().to_int64();
	if (depth.tsNet <= 0) {
		depth.tsNet = GetCurrentTimeUs();
	}
	depth.tsParse = content.at("tsParse").as_number().to_int64();
	if (depth.tsParse <= 0) {
		depth.tsParse = GetCurrentTimeUs();
	}
	depth.tsLocal = GetCurrentTimeUs();

	string key = depth.exchangeType + "|" +  depth.instrumentId + "|" + depth.instrumentType;
	auto iter = mDepth.find(key);
	if (iter != mDepth.end()) {
		iter->second.Add(depth);
	} else {
		DataArray<Depth> dataArr(depthLen);
		dataArr.Add(depth);
		mDepth[key] = dataArr;
	}
}

void BinanceMdMgr::UpdateKline(const web::json::value& content) {
	Kline kline;
	kline.exchangeType = content.at("exchId").as_string();
	kline.instrumentType = content.at("instType").as_string();
	kline.marketType = content.at("marketType").as_string();
	kline.instrumentId = content.at("instId").as_string();
	kline.barTime = content.at("barTime").as_number().to_int64();
	kline.highPrice = content.at("highPrice").as_double();
	kline.lowPrice = content.at("lowPrice").as_double();
	kline.openPrice = content.at("openPrice").as_double();
	kline.closePrice = content.at("closePrice").as_double();
	kline.avgPrice = content.at("avgPrice").as_double();
	kline.totalVolume = content.at("totalVolume").as_double();
	kline.totalAmount = content.at("totalAmount").as_double();
	kline.takerLongVolume = content.at("takerLongVolume").as_double();
	kline.takerLongAmount = content.at("takerLongAmount").as_double();
	kline.takerShortVolume = content.at("takerShortVolume").as_double();
	kline.takerShortAmount = content.at("takerShortAmount").as_double();
	kline.numOfTrade = content.at("numOfTrade").as_integer();
	kline.isFinished = content.at("isFinished").as_string();
	kline.ts = content.at("ts").as_number().to_int64();
	kline.tsNet = content.at("tsNet").as_number().to_int64();
	kline.tsParse = content.at("tsParse").as_number().to_int64();
	kline.tsLocal = GetCurrentTimeUs();

	string key = kline.exchangeType + "|" +  kline.instrumentId + "|" + kline.instrumentType;
	auto iter = mKline.find(key);
	if (iter != mKline.end()) {
		iter->second.Add(kline);
	} else {
		DataArray<Kline> dataArr(klineLen);
		dataArr.Add(kline);
		mKline[key] = dataArr;
	}
}

void BinanceMdMgr::UpdateRate(const web::json::value& content) {
	Rate rate;
	rate.exchangeType = content.at("exchId").as_string();
	rate.instrumentType = content.at("instType").as_string();
	rate.marketType = content.at("marketType").as_string();
	rate.instrumentId = content.at("instId").as_string();
	rate.fundingRate = content.at("fundingRate").as_double();
	rate.nextFundingRate = content.at("nextFundingRate").as_double();
	rate.fundingTime = content.at("fundingTime").as_number().to_int64();
	rate.ts = content.at("ts").as_number().to_int64();
	rate.tsNet = content.at("tsNet").as_number().to_int64();
	rate.tsParse = content.at("tsParse").as_number().to_int64();

	string key = rate.exchangeType + "|" +  rate.instrumentId + "|" + rate.instrumentType;
	auto iter = mRate.find(key);
	if (iter != mRate.end()) {
		iter->second.Add(rate);
	} else {
		DataArray<Rate> dataArr(rateLen);
		dataArr.Add(rate);
		mRate[key] = dataArr;
	}
}

void BinanceMdMgr::UpdateTrade(const web::json::value& content) {
	Trade trade;
	trade.exchangeType = content.at("exchId").as_string();
	trade.instrumentType = content.at("instType").as_string();
	trade.marketType = content.at("marketType").as_string();
	trade.instrumentId = content.at("instId").as_string();
	trade.tradeId = stoll(content.at("tradeId").as_string());
	trade.px = content.at("px").as_double();
	trade.size = content.at("sz").as_double();
	trade.side = content.at("side").as_string();
	trade.ts = content.at("ts").as_number().to_int64();
	trade.tsNet = content.at("tsNet").as_number().to_int64();
	trade.tsParse = content.at("tsParse").as_number().to_int64();

	string key = trade.exchangeType + "|" +  trade.instrumentId + "|" + trade.instrumentType;
	auto iter = mTrade.find(key);
	if (iter != mTrade.end()) {
		iter->second.Add(trade);
	} else {
		DataArray<Trade> dataArr(tradeLen);
		dataArr.Add(trade);
		mTrade[key] = dataArr;
	}
}

Depth BinanceMdMgr::GetLastDepth(string key) {
	Depth lastDepth;
	auto iter = mDepth.find(key);
	if (iter != mDepth.end()) {
		if (!iter->second.IsEmpty()) {
			lastDepth = iter->second.GetEndValue();
		}
	}
	return lastDepth;
}

double BinanceMdMgr::GetMidPrice(string key) {
	double price = 0.0;
	Depth lastDepth = GetLastDepth(key);
	if (lastDepth.askP.size() > 0 && lastDepth.bidP.size() > 0) {
		price = (lastDepth.askP[0] + lastDepth.bidP[0]) / 2;
	}
	return price;
}

double BinanceMdMgr::GetAssetPrice(string asset, string exchangeStr) {
	string key = exchangeStr + "|" + asset + "-" + "USDT" + "|SPOT";
	double price = GetMidPrice(key);
	if (price <= 0.0) {
		key = exchangeStr + "|" + asset + "-" + "USDT" + "|SWAP";
		price = GetMidPrice(key);
	}
	if (price <= 0.0) {
		key = exchangeStr + "|" + asset + "-" + "BUSD" + "|SPOT";
		price = GetMidPrice(key);
		key = exchangeStr + "|" + "BUSD" + "-" + "USDT" + "|SPOT";
		double priceBUSD = GetMidPrice(key);
		price = price * priceBUSD;
	}
	return price;
}

double BinanceMdMgr::GetFundingRate(string key) {
	double fundingRate = 0.0;
	auto iter = mRate.find(key);
	if (iter != mRate.end()) {
		if (!iter->second.IsEmpty()) {
			fundingRate = iter->second.GetEndValue().fundingRate;
		}
	}
	return fundingRate;
}

string BinanceMdMgr::Serialize() {
	web::json::value marketDepthV;
	for (auto iter = mDepth.begin(); iter != mDepth.end(); ++iter) {
		string instrumentKey = iter->first;
		if (iter->second.IsEmpty()) {
			continue;
		}
		
		Depth lastDepth = iter->second.GetEndValue();
		string exchangeTime = CovertToUtcStr(lastDepth.ts);
		string arriveTime = CovertToUtcStr(lastDepth.tsNet);
		string eventTime = CovertToUtcStr(lastDepth.tsLocal);
		double askPrice = 0.0;
		if (lastDepth.askP.size() > 0) {
			askPrice = lastDepth.askP[0];
		}
		double askVolume = 0.0;
		if (lastDepth.askV.size() > 0) {
			askVolume = lastDepth.askV[0];
		}
		double bidPrice = 0.0;
		if (lastDepth.bidP.size() > 0) {
			bidPrice = lastDepth.bidP[0];
		}
		double bidVolume = 0.0;
		if (lastDepth.bidV.size() > 0) {
			bidVolume = lastDepth.bidV[0];
		}

		web::json::value depthV;
		depthV["instrument_key"] = web::json::value::string(instrumentKey);
		depthV["exchange_time"] = web::json::value::string(exchangeTime);
		depthV["arrive_time"] = web::json::value::string(arriveTime);
		depthV["system_time"] = web::json::value::string(eventTime);
		depthV["ask_price"] = web::json::value::number(askPrice);
		depthV["ask_volume"] = web::json::value::number(askVolume);
		depthV["bid_price"] = web::json::value::number(bidPrice);
		depthV["bid_volume"] = web::json::value::number(bidVolume);

		marketDepthV[instrumentKey] = depthV;
	}

	web::json::value mainFormV;
	mainFormV["rm_time"] = web::json::value::string(CovertToUtcStr(GetCurrentTimeUs()));
	mainFormV["value_alarm"] = web::json::value::number(0);
	mainFormV["delay_alarm"] = web::json::value::number(0);
	mainFormV["accumulate_alarm"] = web::json::value::number(0);

	web::json::value listV;
	listV["mainForm"] = mainFormV;
	listV["marketDepth"] = marketDepthV;

	web::json::value dataV;
	dataV["list"] = listV;

	web::json::value res;
	res["type"] = web::json::value::number(3);
	res["data"] = dataV;
	return res.serialize();
}

web::json::value BinanceMdMgr::GetFundingRateValue() {
	web::json::value marketRateV;
	for (auto iter = mRate.begin(); iter != mRate.end(); ++iter) {
		string instrumentKey = iter->first;
		if (iter->second.IsEmpty()) {
			continue;
		}
		
		Rate lastRate = iter->second.GetEndValue();
		double fundingRate = lastRate.fundingRate;
		double nextFundingRate = lastRate.nextFundingRate;
		string fundingTime = CovertToUtcStr(lastRate.fundingTime);
		string exchangeTime = CovertToUtcStr(lastRate.ts);
		web::json::value rateV;
		rateV["instrument_key"] = web::json::value::string(instrumentKey);
		rateV["fundingRate"] = web::json::value::number(fundingRate);
		rateV["nextFundingRate"] = web::json::value::number(nextFundingRate);
		rateV["fundingTime"] = web::json::value::string(fundingTime);
		rateV["exchangeTime"] = web::json::value::string(exchangeTime);

		marketRateV[instrumentKey] = rateV;
	}
	return marketRateV;
}

vector<MsgCard> BinanceMdMgr::GetAlarmMsg() {
	vector<MsgCard> v;
	int64_t currentTime = GetCurrentTimeUs();
	string currentTimeStr = CovertToUtcStr(currentTime);

	vector<string>& vCurrentChannel = MdMonitor::GetInstance().GetCurrentChannel();

	unordered_map<string, string> mTypeMsg;
	for (auto iter = mDepth.begin(); iter != mDepth.end(); ++iter) {
		string instrumentKey = iter->first;
		if (iter->second.IsEmpty()) {
			continue;
		}
		
		Depth lastDepth = iter->second.GetEndValue();
		int64_t exchangeTime = lastDepth.ts;
		if (delayAlarmTime > 0) {
			if (currentTime - exchangeTime >= delayAlarmTime * 60 * 1000 * 1000) {
				string channel = lastDepth.exchangeType + "." + lastDepth.instrumentType + "." + lastDepth.marketType + "." + lastDepth.instrumentId;
				for (size_t i = 0; i < vCurrentChannel.size(); ++i) {
					if (vCurrentChannel[i] == channel) {
						mTypeMsg[lastDepth.instrumentType] += "【" + instrumentKey + "|" + lastDepth.marketType + "】";
						break;
					}
				}
			}	
		}
	}

	for (auto iter = mTypeMsg.begin(); iter != mTypeMsg.end(); ++iter) {
		stringstream ss;
		ss << "行情" << iter->second << "date-systemtime大于等于" <<  delayAlarmTime << "min";
    	MsgCard msgCard;
    	msgCard.templateId = 3;
    	msgCard.title = "date-systemtime";
    	msgCard.object = "账户：行情系统";
    	msgCard.datetime = currentTimeStr;
    	msgCard.content = ss.str();
		v.emplace_back(msgCard);
	}

	return v;
}

vector<MsgCard> BinanceMdMgr::GetKlineAlarmMsg() {
	vector<MsgCard> v;
	int64_t currentTime = GetCurrentTimeUs();
	string currentTimeStr = CovertToUtcStr(currentTime);

	vector<string>& vCurrentChannel = MdMonitor::GetInstance().GetCurrentChannel();

	unordered_map<string, string> mTypeMsg;
	for (auto iter = mKline.begin(); iter != mKline.end(); ++iter) {
		string instrumentKey = iter->first;
		if (iter->second.IsEmpty()) {
			continue;
		}

		Kline lastKline = iter->second.GetEndValue();
		int64_t exchangeTime = lastKline.ts;
		if (delayKlineAlarmTime > 0) {
			if (currentTime - exchangeTime >= delayKlineAlarmTime * 60 * 1000 * 1000) {
				string channel = lastKline.exchangeType + "." + lastKline.instrumentType + "." + lastKline.marketType + "." + lastKline.instrumentId;
				for (size_t i = 0; i < vCurrentChannel.size(); ++i) {
					if (vCurrentChannel[i] == channel) {
						mTypeMsg[lastKline.instrumentType] += "【" + instrumentKey + "|" + lastKline.marketType + "】";
						break;
					}
				}
			}	
		}
	}

	for (auto iter = mTypeMsg.begin(); iter != mTypeMsg.end(); ++iter) {
		stringstream ss;
		ss << "行情" << iter->second << "date-systemtime大于等于" <<  delayKlineAlarmTime << "min";
    	MsgCard msgCard;
    	msgCard.templateId = 3;
    	msgCard.title = "date-systemtime";
    	msgCard.object = "账户：行情系统";
    	msgCard.datetime = currentTimeStr;
    	msgCard.content = ss.str();
		v.emplace_back(msgCard);
	}

	return v;
}

vector<MsgCard> BinanceMdMgr::GetTradeAlarmMsg() {
	vector<MsgCard> v;
	int64_t currentTime = GetCurrentTimeUs();
	string currentTimeStr = CovertToUtcStr(currentTime);

	vector<string>& vCurrentChannel = MdMonitor::GetInstance().GetCurrentChannel();

	unordered_map<string, string> mTypeMsg;
	for (auto iter = mTrade.begin(); iter != mTrade.end(); ++iter) {
		string instrumentKey = iter->first;
		if (iter->second.IsEmpty()) {
			continue;
		}

		Trade lastTrade = iter->second.GetEndValue();
		int64_t exchangeTime = lastTrade.ts;
		if (delayTradeAlarmTime > 0) {
			if (currentTime - exchangeTime >= delayTradeAlarmTime * 60 * 1000 * 1000) {
				string channel = lastTrade.exchangeType + "." + lastTrade.instrumentType + "." + lastTrade.marketType + "." + lastTrade.instrumentId;
				for (size_t i = 0; i < vCurrentChannel.size(); ++i) {
					if (vCurrentChannel[i] == channel) {
						mTypeMsg[lastTrade.instrumentType] += "【" + instrumentKey + "|" + lastTrade.marketType + "】";
						break;
					}
				}
			}	
		}
	}

	for (auto iter = mTypeMsg.begin(); iter != mTypeMsg.end(); ++iter) {
		stringstream ss;
		ss << "行情" << iter->second << "date-systemtime大于等于" <<  delayTradeAlarmTime << "min";
    	MsgCard msgCard;
    	msgCard.templateId = 3;
    	msgCard.title = "date-systemtime";
    	msgCard.object = "账户：行情系统";
    	msgCard.datetime = currentTimeStr;
    	msgCard.content = ss.str();
		v.emplace_back(msgCard);
	}

	return v;
}

vector<string> BinanceMdMgr::GetCurrentMd() {
	vector<string> v;
	for (auto iter = mDepth.begin(); iter != mDepth.end(); ++iter) {
		string instrumentKey = iter->first;
		if (iter->second.IsEmpty()) {
			continue;
		}
		
		Depth lastDepth = iter->second.GetEndValue();
		string key = instrumentKey + "|" + lastDepth.marketType;
		v.emplace_back(key);
	}

	for (auto iter = mKline.begin(); iter != mKline.end(); ++iter) {
		string instrumentKey = iter->first;
		if (iter->second.IsEmpty()) {
			continue;
		}

		Kline lastKline = iter->second.GetEndValue();
		string key = instrumentKey + "|" + lastKline.marketType;
		v.emplace_back(key);
	}

	return v;
}

vector<MsgCard> BinanceMdMgr::GetMdPriceVoiceCallMsg() {
	vector<MsgCard> v;
	int64_t currentTime = GetCurrentTimeUs();
	string currentTimeStr = CovertToUtcStr(currentTime);
	for (size_t i = 0; i < vMdPriceVoiceCall.size(); ++i) {
		string symbol = vMdPriceVoiceCall[i].symbol;
		string key = "BINANCE|" + symbol + "|SPOT";
		double price = GetMidPrice(key);
		if (price <= 0.0) {
			continue;
		}
		
		if (price <= vMdPriceVoiceCall[i].price && vMdPriceVoiceCall[i].mode <= 0) {
			stringstream ss;
			ss << "行情【" << key << "】价格小于等于" <<  vMdPriceVoiceCall[i].price << " 当前价格为" << price;
			MsgCard msgCard;
        	msgCard.templateId = 1;
        	msgCard.title = "价格预警";
        	msgCard.object = "账户：行情系统";
        	msgCard.datetime = currentTimeStr;
        	msgCard.content = ss.str();
			v.emplace_back(msgCard);
		}
		else if (price > vMdPriceVoiceCall[i].price && vMdPriceVoiceCall[i].mode > 0) {
			stringstream ss;
			ss << "行情【" << key << "】价格大于" <<  vMdPriceVoiceCall[i].price << " 当前价格为" << price;
			MsgCard msgCard;
        	msgCard.templateId = 1;
        	msgCard.title = "价格预警";
        	msgCard.object = "账户：行情系统";
        	msgCard.datetime = currentTimeStr;
        	msgCard.content = ss.str();
			v.emplace_back(msgCard);
		}
	}

	return v;
}

vector<MsgCard> BinanceMdMgr::GetExchangeDisconnectVoiceCallMsg() {
	set<string> sBinanceSpot = MonitorConfig::GetInstance().GetBinanceDisconnectSpot();
    set<string> sBinanceUfuture = MonitorConfig::GetInstance().GetBinanceDisconnectUfuture();
    set<string> sBinanceCfuture = MonitorConfig::GetInstance().GetBinanceDisconnectCfuture();
    set<string> sGateioSpot = MonitorConfig::GetInstance().GetGateioDisconnectSpot();
    set<string> sGateioSwap = MonitorConfig::GetInstance().GetGateioDisconnectSwap();

	vector<MsgCard> v;
	int64_t currentTime = GetCurrentTimeUs();
	string currentTimeStr = CovertToUtcStr(currentTime);

	for (auto iter = mDepth.begin(); iter != mDepth.end(); ++iter) {
		string instrumentKey = iter->first;
		if (iter->second.IsEmpty()) {
			continue;
		}
		
		Depth lastDepth = iter->second.GetEndValue();
		int64_t exchangeTime = lastDepth.ts;
		if (delayAlarmTime > 0) {
			if (currentTime - exchangeTime >= delayAlarmTime * 60 * 1000 * 1000) {
				sBinanceSpot.erase(instrumentKey);
				sBinanceUfuture.erase(instrumentKey);
				sBinanceCfuture.erase(instrumentKey);
				sGateioSpot.erase(instrumentKey);
				sGateioSwap.erase(instrumentKey);
			}	
		}
	}

	if (sBinanceSpot.size() == 0) {
		stringstream ss;
		ss << "Binance Exchange Spot cannot connect!";
    	MsgCard msgCard;
    	msgCard.templateId = 1;
    	msgCard.title = "Binance Exchange Failed";
    	msgCard.object = "账户：交易所";
    	msgCard.datetime = currentTimeStr;
    	msgCard.content = ss.str();
		v.emplace_back(msgCard);
	}

	if (sBinanceUfuture.size() == 0) {
		stringstream ss;
		ss << "Binance Exchange Ufuture cannot connect!";
    	MsgCard msgCard;
    	msgCard.templateId = 1;
    	msgCard.title = "Binance Exchange Failed";
    	msgCard.object = "账户：交易所";
    	msgCard.datetime = currentTimeStr;
    	msgCard.content = ss.str();
		v.emplace_back(msgCard);
	}

	if (sBinanceCfuture.size() == 0) {
		stringstream ss;
		ss << "Binance Exchange Cfuture cannot connect!";
    	MsgCard msgCard;
    	msgCard.templateId = 1;
    	msgCard.title = "Binance Exchange Failed";
    	msgCard.object = "账户：交易所";
    	msgCard.datetime = currentTimeStr;
    	msgCard.content = ss.str();
		v.emplace_back(msgCard);
	}

	if (sGateioSpot.size() == 0) {
		stringstream ss;
		ss << "Gateio Exchange Spot cannot connect!";
    	MsgCard msgCard;
    	msgCard.templateId = 1;
    	msgCard.title = "Gateio Exchange Failed";
    	msgCard.object = "账户：交易所";
    	msgCard.datetime = currentTimeStr;
    	msgCard.content = ss.str();
		v.emplace_back(msgCard);
	}

	if (sGateioSwap.size() == 0) {
		stringstream ss;
		ss << "Gateio Exchange Swap cannot connect!";
    	MsgCard msgCard;
    	msgCard.templateId = 1;
    	msgCard.title = "Gateio Exchange Failed";
    	msgCard.object = "账户：交易所";
    	msgCard.datetime = currentTimeStr;
    	msgCard.content = ss.str();
		v.emplace_back(msgCard);
	}

	return v;
}
