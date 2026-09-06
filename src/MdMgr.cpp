#include "MdMgr.h"
#include "MonitorConfig.h"
#include "MdMonitor.h"
#include "ExchangeRestMd.h"

MdMgr::MdMgr() {
	depthLen = 10;
}

MdMgr::~MdMgr() {
	mDepth.clear();
}

MdMgr& MdMgr::GetInstance() {
	static MdMgr mdMgr;
	return mdMgr;
}

void MdMgr::UpdateFromRest() {
    std::set<std::string> sInstrumentBinanceAdapter = BinanceAdapterMgr::GetInstance().GetInstrumentList();
    std::set<std::string> sInstrumentGateioAdapter = GateioAdapterMgr::GetInstance().GetInstrumentList();
    std::set<std::string> sInstrumentBybitAdapter = BybitAdapterMgr::GetInstance().GetInstrumentList();
    std::set<std::string> sInstrumentOkxAdapter = OkxAdapterMgr::GetInstance().GetInstrumentList();

	std::set<std::string> s;
	s.insert(sInstrumentBinanceAdapter.begin(), sInstrumentBinanceAdapter.end());
    s.insert(sInstrumentGateioAdapter.begin(), sInstrumentGateioAdapter.end());
    s.insert(sInstrumentBybitAdapter.begin(), sInstrumentBybitAdapter.end());
    s.insert(sInstrumentOkxAdapter.begin(), sInstrumentOkxAdapter.end());


	for (auto it = s.begin(); it != s.end(); ++it) {
		std::string channel = *it;
		std::vector<std::string> v;
        SplitString(channel, ".", v);
		std::string exchangeType = v[0];
		std::string instType = v[1];
		std::string instId = v[3];
		
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

Depth MdMgr::GetLastDepth(string key) {
	Depth lastDepth;
	auto iter = mDepth.find(key);
	if (iter != mDepth.end()) {
		if (!iter->second.IsEmpty()) {
			lastDepth = iter->second.GetEndValue();
		}
	}
	return lastDepth;
}

double MdMgr::GetMidPrice(string key) {
	double price = 0.0;
	Depth lastDepth = GetLastDepth(key);
	if (lastDepth.askP.size() > 0 && lastDepth.bidP.size() > 0) {
		price = (lastDepth.askP[0] + lastDepth.bidP[0]) / 2;
	}
	return price;
}

double MdMgr::GetAssetPrice(string asset, string exchangeStr) {
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
