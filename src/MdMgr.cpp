#include "MdMgr.h"
#include "MonitorConfig.h"
#include "MdMonitor.h"
#include "ExchangeRestMd.h"

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
