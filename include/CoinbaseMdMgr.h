#pragma once

#include <cpprest/json.h>
#include <string>
#include <map>
#include <queue>
#include "DataArray.h"
#include "Utility.h"

using namespace std;


class CoinbaseMdMgr {
public:
	static CoinbaseMdMgr& GetInstance();
	~CoinbaseMdMgr();
    void GetMd();
	Depth GetLastDepth(string key);
	double GetMidPrice(string key);
	double GetAssetPrice(string asset);  // 获取对应asset现货的价格（u计价），若没有则获取永续合约的价格（u计价）
	string Serialize();
	void AddNewProduct(set<string> s);

private:
	CoinbaseMdMgr();
    string tickerUrl;
    vector<string> vProductList;
	set<string> sProductList;
	unordered_map<string, DataArray<Depth>> mDepth;
	unordered_map<string, DataArray<Kline>> mKline;

	int depthLen;
	int klineLen;
};
