#pragma once

#include <string>
#include <map>
#include <queue>
#include "DataArray.h"
#include "Utility.h"

using namespace std;


class MdMgr {
public:
	static MdMgr& GetInstance();
	~MdMgr();

	void UpdateFromRest();
	Depth GetLastDepth(string key);
	double GetMidPrice(string key);
	double GetAssetPrice(string asset, string exchangeStr);  // 获取对应asset现货的价格（u计价），若没有则获取永续合约的价格（u计价）

private:
	MdMgr();

	unordered_map<string, DataArray<Depth>> mDepth;

	int depthLen;
};
