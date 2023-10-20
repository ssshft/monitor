#include "CoinbaseMdMgr.h"
#include "MonitorConfig.h"

CoinbaseMdMgr::CoinbaseMdMgr() {
    tickerUrl = "https://api.exchange.coinbase.com/products/";
	depthLen = 10;
	klineLen = 10;
	//vProductList = MonitorConfig::GetInstance().GetCoinbaseMdSymbol();
}

CoinbaseMdMgr::~CoinbaseMdMgr() {
	mDepth.clear();
	mKline.clear();
}

CoinbaseMdMgr& CoinbaseMdMgr::GetInstance() {
	static CoinbaseMdMgr coinbaseMdMgr;
	return coinbaseMdMgr;
}

void CoinbaseMdMgr::GetMd() {
    for (auto iter = sProductList.begin(); iter != sProductList.end(); ++iter) {
		string product = *iter;
        try {
            string url = tickerUrl + product + "/ticker";
		    web::http::client::http_client_config config;
            config.set_timeout(utility::seconds(5));
		    web::http::client::http_client restclient(url, config);
    	    web::http::http_request request(web::http::methods::GET);

    	    restclient.request(request).then([](web::http::http_response response) -> pplx::task<web::json::value> {
			    auto code = response.status_code();
        	    if (code == web::http::status_codes::OK || code == web::http::status_codes::BadRequest || code == web::http::status_codes::TooManyRequests || code == web::http::status_codes::Unauthorized) {
                    return response.extract_json();
			    }
        	    throw exception();
        	    return pplx::task_from_result(web::json::value());
            })
            .then([&](pplx::task<web::json::value> previousTask) {
                Depth depth;
                depth.instrumentId = product;
			    web::json::value const& content = previousTask.get();
                if (content.has_field("bid")) {
                    double bidPrice = stod(content.at("bid").as_string());
                    depth.bidP.push_back(bidPrice);
                }
                if (content.has_field("ask")) {
                    double askPrice = stod(content.at("ask").as_string());
                    depth.askP.push_back(askPrice);
                }

	            string key = "COINBASE|" +  product + "|SPOT";
	            auto iter = mDepth.find(key);
	            if (iter != mDepth.end()) {
		            iter->second.Add(depth);
	            } else {
		            DataArray<Depth> dataArr(depthLen);
		            dataArr.Add(depth);
		            mDepth[key] = dataArr;
	            }
            })
            .wait();
	    } catch(exception e) {
            std::cerr << "CoinbaseMdMgr GetMd error: " << e.what() << std::endl;
        } 
    }
}

Depth CoinbaseMdMgr::GetLastDepth(string key) {
	Depth lastDepth;
	auto iter = mDepth.find(key);
	if (iter != mDepth.end()) {
		if (!iter->second.IsEmpty()) {
			lastDepth = iter->second.GetEndValue();
		}
	}
	return lastDepth;
}

double CoinbaseMdMgr::GetMidPrice(string key) {
	double price = 0.0;
	Depth lastDepth = GetLastDepth(key);
	if (lastDepth.askP.size() > 0 && lastDepth.bidP.size() > 0) {
		price = (lastDepth.askP[0] + lastDepth.bidP[0]) / 2;
	}
	return price;
}

double CoinbaseMdMgr::GetAssetPrice(string asset) {
	string key = "COINBASE|" + asset + "-" + "USD" + "|SPOT";
	double price; 
	if (asset == "USDC" || asset == "USDT" || asset == "USD") {
		price = 1;
	} else {
		price = GetMidPrice(key);
	}
	return price;
}

string CoinbaseMdMgr::Serialize() {
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

void CoinbaseMdMgr::AddNewProduct(set<string> s) {
	sProductList.insert(s.begin(), s.end());
}
