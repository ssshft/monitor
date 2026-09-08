#pragma once

#include <string>
#include <unordered_map>
#include "Utility.h"


class MdMgr {
public:
	static MdMgr& GetInstance();
	~MdMgr();

	void UpdateFromRest();
	Depth GetLastDepth(const std::string& key);
	double GetMidPrice(const std::string& key);
	double GetAssetPrice(const std::string& asset, const std::string& exchangeStr);  // 获取对应asset现货的价格（u计价），若没有则获取永续合约的价格（u计价）

private:
	MdMgr();
	unordered_map<string, Depth> mDepth;
};
