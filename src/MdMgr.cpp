#include "MdMgr.h"
#include "MonitorConfig.h"
#include "ExchangeRestMd.h"
#include "BinanceAdapterMgr.h"
#include "GateioAdapterMgr.h"
#include "BybitAdapterMgr.h"
#include "OkxAdapterMgr.h"


MdMgr::MdMgr() {

}

MdMgr::~MdMgr() {
	mDepth.clear();
}

MdMgr& MdMgr::GetInstance() {
	static MdMgr mdMgr;
	return mdMgr;
}

void MdMgr::UpdateFromRest() {
    std::unordered_map<std::string, md::InstrumentInfo> mInstrumentBinanceAdapter = BinanceAdapterMgr::GetInstance().GetInstrumentList();
    std::unordered_map<std::string, md::InstrumentInfo> mInstrumentGateioAdapter = GateioAdapterMgr::GetInstance().GetInstrumentList();
    std::unordered_map<std::string, md::InstrumentInfo> mInstrumentBybitAdapter = BybitAdapterMgr::GetInstance().GetInstrumentList();
    std::unordered_map<std::string, md::InstrumentInfo> mInstrumentOkxAdapter = OkxAdapterMgr::GetInstance().GetInstrumentList();

	std::unordered_map<std::string, md::InstrumentInfo> m;
	for (auto& [key, value] : mInstrumentBinanceAdapter) {
		m[key] = value;
	}
	for (auto& [key, value] : mInstrumentGateioAdapter) {
		m[key] = value;
	}
	for (auto& [key, value] : mInstrumentBybitAdapter) {
		m[key] = value;
	}
	for (auto& [key, value] : mInstrumentOkxAdapter) {
		m[key] = value;
	}

	for (auto iter = m.begin(); iter != m.end(); ++iter) {
		std::string key = iter->first;
		const Depth& depth = ExchangeRestMd::GetInstance().GetDepth(iter->second.exchangeTypeEnum, iter->second.instTypeEnum, iter->second.originInstId);
		if (depth.ts > 0) {
			mDepth[key] = depth;
		}
		usleep(100000);
	}
}

Depth MdMgr::GetLastDepth(const std::string& key) {
	Depth lastDepth;
	auto iter = mDepth.find(key);
	if (iter != mDepth.end()) {
		lastDepth = iter->second;
	}
	return lastDepth;
}

double MdMgr::GetMidPrice(const std::string&  key) {
	double price = 0.0;
	const Depth& lastDepth = GetLastDepth(key);
	if (lastDepth.askP.size() > 0 && lastDepth.bidP.size() > 0) {
		price = (lastDepth.askP[0] + lastDepth.bidP[0]) / 2;
	}
	return price;
}

double MdMgr::GetAssetPrice(const std::string&  asset, const std::string&  exchangeStr) {
	std::string instId = asset + "-" + "USDT";
	std::string key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], SPOT, instId);
	double price = GetMidPrice(key);
	if (price <= 0.0) {
		key = crypto::get_instrumentInfo_channel_key(ExchangeTypeStr2EnumMap[exchangeStr], USDT_SWAP, instId);
		price = GetMidPrice(key);
	}
	if (price <= 0.0) {
		LOG_ERROR("cannot get asset price, asset:{} exchange:{}", asset, exchangeStr);
	}
	return price;
}
