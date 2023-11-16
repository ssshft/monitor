#include "ProductItem.h"
#include "BinanceAccountMgr.h"
#include "MonitorConfig.h"
#include "BinanceMdMgr.h"
#include "CoinbaseMdMgr.h"
#include "BinanceAdapterMgr.h"
#include "CoinbaseAdapterMgr.h"
#include "GateioAdapterMgr.h"
#include "BybitAdapterMgr.h"
#include <regex>

ProductItem::ProductItem(string na, vector<int> v) {
    name = na;
    vAccountId = v;
    baseAsset = MonitorConfig::GetInstance().GetProductBaseAsset(name);
    riskInfo.accountId = MonitorConfig::GetInstance().GetProductId(name);
    riskInfo.name = name;
    riskInfo.baseAsset = baseAsset;
    alarmInfo = MonitorConfig::GetInstance().GetProductAlarmInfo(name);
    adapterQuery = true;
}

ProductItem::~ProductItem() {
    vAccountId.clear();
    mSpotAsset.clear();
    mUFutureAsset.clear();
    mCFutureAsset.clear();
    mExposure.clear();
}

void ProductItem::UpdateAsset(unordered_map<string, igmonitor::Asset>& source, unordered_map<string, igmonitor::Asset>& target) {
    for (auto iter = source.begin(); iter != source.end(); ++iter) {
        auto it = target.find(iter->first);
        if (it != target.end()) {
            it->second.positionValueS += iter->second.positionValueS;
            it->second.positionValueD += iter->second.positionValueD;
            it->second.transferAmount += iter->second.transferAmount;
            it->second.transferFrozenAmount += iter->second.transferFrozenAmount;
            it->second.netAmountS += iter->second.netAmountS;
            it->second.netAmountD += iter->second.netAmountD;
            it->second.availableAmountS += iter->second.availableAmountS;
            it->second.availableAmountD += iter->second.availableAmountD;
            it->second.totalAmountS += iter->second.totalAmountS;
            it->second.totalAmountD += iter->second.totalAmountD;
            it->second.frozenAmountS += iter->second.frozenAmountS;
            it->second.frozenAmountD += iter->second.frozenAmountD;
            it->second.floatAmountS += iter->second.floatAmountS;
            it->second.floatAmountD += iter->second.floatAmountD;
            it->second.marginAmountS += iter->second.marginAmountS;
            it->second.marginAmountD += iter->second.marginAmountD;
            it->second.frozenMarginAmountS += iter->second.frozenMarginAmountS;
            it->second.frozenMarginAmountD += iter->second.frozenMarginAmountD;
            it->second.underwayOrderValueS += iter->second.underwayOrderValueS;
            it->second.underwayOrderValueD += iter->second.underwayOrderValueD;
            it->second.initialAmount += iter->second.initialAmount;
            if (fabs(it->second.netAmountS) > 0) {
                it->second.realLeverageRatioS = fabs(it->second.positionValueS / it->second.netAmountS);
            }
            if (fabs(it->second.netAmountD) > 0) {
                it->second.realLeverageRatioD = fabs(it->second.positionValueD / it->second.netAmountD);
            }
        } else {
            target.insert(make_pair(iter->first, iter->second));
        }
    }
}

void ProductItem::CalculateRiskInfo() {
    sSymbolPriceZero.clear();
    mExposure.clear();
    mSpotAsset.clear();
    mUFutureAsset.clear();
    mCFutureAsset.clear();
    mPositionFundingRate.clear();
    adapterQuery = true;
    double netValue = 0.0;
    double maxRealLeverageD = 0.0;
    for (size_t i = 0; i < vAccountId.size(); ++i) {
        int accountId = vAccountId[i];
        BinanceAccountItem* item = BinanceAccountMgr::GetInstance().GetAccountItem(accountId);
        if (item) {
            bool query = item->GetAdapterQueryStatus();
            adapterQuery = adapterQuery && query;
            exchangeStr = item->GetExchangeStr();

            unordered_map<string, igmonitor::Exposure>& exposure = item->GetExposure();  // exposure 获取每个账户的mExposure，将对应asset的exposure相加
            for (auto iter = exposure.begin(); iter != exposure.end(); ++iter) {
                smatch result;
                regex e("^[0-9]+");
                string pre = "";
                if (regex_search(iter->first, result, e)) {
                    pre = result.str();
                }
                int preNum = 1;
                if (pre.length() > 0) {
                    preNum = stoi(pre);
                }
                string ass = iter->first.substr(pre.length(), iter->first.length());
                if (fabs(iter->second.exposureValueD) < 0.000000001) {
                    sSymbolPriceZero.insert(ass);
                }

                stringstream ss;
                ss << "Exposure: " << riskInfo.name << "accountId:" << accountId << "ass:" << ass << " asset: " << iter->first << " preNum:" << preNum << " amount:" << iter->second.exposureAmountD << " value:" << iter->second.exposureValueD;
                LOG_INFO("Exposure: %s", ss.str().c_str()); 

                auto it = mExposure.find(ass);
                if (it != mExposure.end()) {
                    it->second.initialAmount += iter->second.initialAmount;
                    it->second.initialValue += iter->second.initialValue;
                    it->second.exposureAmountS += iter->second.exposureAmountS;
                    it->second.exposureAmountD += iter->second.exposureAmountD * preNum;
                    it->second.exposureValueS += iter->second.exposureValueS;
                    it->second.exposureValueD += iter->second.exposureValueD;
                    it->second.deltaAmountS += iter->second.deltaAmountS;
                    it->second.deltaAmountD += iter->second.deltaAmountD;
                    it->second.deltaValueS += iter->second.deltaValueS;
                    it->second.deltaValueD += iter->second.deltaValueD;
                } else {
		            igmonitor::Exposure expo = iter->second;
		            expo.exposureAmountD *= preNum;
                    mExposure.insert(make_pair(ass, expo));
                }

            }

            igmonitor::RiskInfo& info = item->GetRiskInfo();
            if (info.maxRealLeverageD > maxRealLeverageD) {
                maxRealLeverageD = info.maxRealLeverageD;
            }

            // underwayordervalue
            unordered_map<string, igmonitor::Asset>& spot = item->GetSpotAsset();
            UpdateAsset(spot, mSpotAsset);
            unordered_map<string, igmonitor::Asset>& ufuture = item->GetUFutureAsset();
            UpdateAsset(ufuture, mUFutureAsset);
            unordered_map<string, igmonitor::Asset>& cfuture = item->GetCFutureAsset();
            UpdateAsset(cfuture, mCFutureAsset);

            // funding rate
            unordered_map<string, igmonitor::PositionFundingRate>& mFr = item->GetPositionFundingRate();
            for (auto m = mFr.begin(); m != mFr.end(); ++m) {
                string key = m->first;
                vector<string> v;
                SplitString(key, "|", v);
                if (v.size() >= 3) {
                    string instId = v[1];
                    smatch result;
                    regex e("^[0-9]+");
                    string pre = "";
                    if (regex_search(instId, result, e)) {
                        pre = result.str();
                    }
                    string ass = instId.substr(pre.length(), instId.length());
                    auto n = mPositionFundingRate.find(ass);
                    if (n != mPositionFundingRate.end()) {
                        if (exchangeStr == "GATEIO") {
                            n->second.flag &= !m->second.flag;
                            n->second.value += -m->second.value;
                        } else if (exchangeStr == "BINANCE") {
                            n->second.flag &= m->second.flag;
                            n->second.value += m->second.value; 
                        }
                        LOG_INFO("UpdatePositionFundingRate:  name: %s  ass:%s  flag:%d  value:%f", riskInfo.name.c_str(), ass.c_str(), n->second.flag, n->second.value);
                    } else {
                        if (exchangeStr == "GATEIO") {
                            igmonitor::PositionFundingRate pfr;
                            pfr.symbol = ass;
                            pfr.flag = !m->second.flag;
                            pfr.value = -m->second.value;
                            mPositionFundingRate[ass] = pfr;
                        } else if (exchangeStr == "BINANCE") {
                            igmonitor::PositionFundingRate pfr;
                            pfr.symbol = ass;
                            pfr.flag = m->second.flag;
                            pfr.value = m->second.value;
                            mPositionFundingRate[ass] = pfr;
                        }
                    }
                }
            }

            // netValue
            netValue += info.netValueD;
        }
    }
    riskInfo.maxRealLeverageD = maxRealLeverageD;

    double riskExposureS = 0.0;
    double riskExposureD = 0.0;
    for (auto iter = mExposure.begin(); iter != mExposure.end(); ++iter) {
        if (iter->first == baseAsset) {
            continue;
        }

        double price = 0.0;
        if (iter->first == "USDT" || iter->first == "BUSD") {
            price = 1;
        } else {
            price = BinanceMdMgr::GetInstance().GetAssetPrice(iter->first, "GATEIO");
        }
        
        double priceBaseAsset = 1;
        if (baseAsset == "USDT" || baseAsset == "USD") {
            priceBaseAsset = 1;
        } else {
            priceBaseAsset = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, "GATEIO");
        }
        
        double amount = iter->second.exposureAmountD;
        double value = 0.0;
        if (priceBaseAsset > 0) {
            value = fabs(amount * price / priceBaseAsset);
        }

        stringstream ss;
        ss << "Exposure: " << riskInfo.name << " asset: " << iter->first << " amount:" << iter->second.exposureAmountD << " price:" << price << " priceBaseAsset:" << priceBaseAsset << " value:" << value;
        LOG_INFO("Exposure: %s", ss.str().c_str()); 

        if (fabs(value) > fabs(riskExposureD)) { // exposureAmountD 相加 然后 * price
            maxRiskExposureAssetD = iter->first;
            riskExposureD = value;
        }

        /*
        if (fabs(iter->second.exposureValueD) > fabs(riskExposureD)) { // exposureAmountD 相加 然后 * price
            riskExposureD = iter->second.exposureValueD;
        }
        */

        if (fabs(iter->second.exposureValueS) > fabs(riskExposureS)) {
            riskExposureS = iter->second.exposureValueS;
        }
    }
    riskInfo.riskExposureS = riskExposureS;
    riskInfo.riskExposureD = riskExposureD;

    double underwayOrderValueD = 0.0;
    for (auto iter = mSpotAsset.begin(); iter != mSpotAsset.end(); ++iter) {
        if (iter->second.underwayOrderValueD > underwayOrderValueD) {
            underwayOrderValueD = iter->second.underwayOrderValueD;
        }
    }

    for (auto iter = mUFutureAsset.begin(); iter != mUFutureAsset.end(); ++iter) {
        if (iter->second.underwayOrderValueD > underwayOrderValueD) {
            underwayOrderValueD = iter->second.underwayOrderValueD;
        }
    }

    for (auto iter = mCFutureAsset.begin(); iter != mCFutureAsset.end(); ++iter) {
        if (iter->second.underwayOrderValueD > underwayOrderValueD) {
            underwayOrderValueD = iter->second.underwayOrderValueD;
        }
    }
    riskInfo.underwayOrderValueD = underwayOrderValueD;
    riskInfo.netValueD = netValue;

    LOG_INFO("riskInfo: %s", riskInfo.toString().c_str()); 
}

vector<MsgCard> ProductItem::GetAlarmMsg() {
    vector<MsgCard> v;

    if (adapterQuery) {
        int64_t currentTime = gettickcount();

        if (alarmInfo.leverageThreshold.warning > 0 && alarmInfo.leverageThreshold.alarm >= alarmInfo.leverageThreshold.warning) {
            bool reachWarning = true;
            bool reachAlarm = true;

            if (riskInfo.maxRealLeverageD < alarmInfo.leverageThreshold.warning) {
                reachWarning = false;
                reachAlarm = false;
            } else if (riskInfo.maxRealLeverageD >= alarmInfo.leverageThreshold.warning && riskInfo.maxRealLeverageD < alarmInfo.leverageThreshold.alarm) {
                reachAlarm = false;
            }
     
            if (reachAlarm) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "Leverage大于等于" <<  alarmInfo.leverageThreshold.alarm << " 当前leverage=" << riskInfo.maxRealLeverageD;

                MsgCard msgCard;
                msgCard.name = name;
                msgCard.templateId = 3;
                msgCard.title = "Leverage";
                msgCard.object = "账户：" + name;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "Leverage大于等于" <<  alarmInfo.leverageThreshold.warning << " 当前leverage=" << riskInfo.maxRealLeverageD;

                MsgCard msgCard;
                msgCard.name = name;
                msgCard.templateId = 2;
                msgCard.title = "Leverage";
                msgCard.object = "账户：" + name;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }

        if (alarmInfo.riskExposureThreshold.warning > 0 && alarmInfo.riskExposureThreshold.alarm >= alarmInfo.riskExposureThreshold.warning) {
            bool reachWarning = true;
            bool reachAlarm = true;

            if (fabs(riskInfo.riskExposureD) < alarmInfo.riskExposureThreshold.warning) {
                reachWarning = false;
                reachAlarm = false;
            } else if (fabs(riskInfo.riskExposureD) >= alarmInfo.riskExposureThreshold.warning && fabs(riskInfo.riskExposureD) < alarmInfo.riskExposureThreshold.alarm) {
                reachAlarm = false;
            }

            if (reachAlarm) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.alarm << " 当前riskexposure=" << riskInfo.riskExposureD << " symbol:" << maxRiskExposureAssetD;

                MsgCard msgCard;
                msgCard.name = name;
                msgCard.templateId = 3;
                msgCard.title = "RiskExposure";
                msgCard.object = "账户：" + name;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "RiskExposure大于等于" <<  alarmInfo.riskExposureThreshold.warning << " 当前riskexposure=" << riskInfo.riskExposureD << " symbol:" << maxRiskExposureAssetD;

                MsgCard msgCard;
                msgCard.name = name;
                msgCard.templateId = 2;
                msgCard.title = "RiskExposure";
                msgCard.object = "账户：" + name;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }


        if (alarmInfo.underwayOrderValueThreshold.warning > 0 && alarmInfo.underwayOrderValueThreshold.alarm >= alarmInfo.underwayOrderValueThreshold.warning) {
            bool reachWarning = true;
            bool reachAlarm = true;

            if (fabs(riskInfo.underwayOrderValueD) < alarmInfo.underwayOrderValueThreshold.warning) {
                reachWarning = false;
                reachAlarm = false;
            } else if (fabs(riskInfo.underwayOrderValueD) >= alarmInfo.underwayOrderValueThreshold.warning && fabs(riskInfo.underwayOrderValueD) < alarmInfo.underwayOrderValueThreshold.alarm) {
                reachAlarm = false;
            }
            
            if (reachAlarm) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "UnderwayOrderValueThreshold大于等于" <<  alarmInfo.underwayOrderValueThreshold.alarm << " 当前underwayordervalue=" << riskInfo.underwayOrderValueD;

                MsgCard msgCard;
                msgCard.name = name;
                msgCard.templateId = 3;
                msgCard.title = "UnderwayOrderValueThreshold";
                msgCard.object = "账户：" + name;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            } else if (reachWarning) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "UnderwayOrderValueThreshold大于等于" <<  alarmInfo.underwayOrderValueThreshold.warning << " 当前underwayordervalue=" << riskInfo.underwayOrderValueD;

                MsgCard msgCard;
                msgCard.name = name;
                msgCard.templateId = 2;
                msgCard.title = "UnderwayOrderValueThreshold";
                msgCard.object = "账户：" + name;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }

        if (alarmInfo.netValueThreshold.monthValue > 0 && alarmInfo.netValueThreshold.initValue > 0) {
            double netValueU = riskInfo.netValueD;
            if (baseAsset != "USDT") {
                double price = 0.0;
                if (exchangeStr == "BINANCE" || exchangeStr == "GATEIO" || exchangeStr == "BYBIT") {
                    price = BinanceMdMgr::GetInstance().GetAssetPrice(baseAsset, exchangeStr);
                } else if (exchangeStr == "COINBASE") {
                    price = CoinbaseMdMgr::GetInstance().GetAssetPrice(baseAsset);
                }
                netValueU *= price;
            }
            double maxValue = max(alarmInfo.netValueThreshold.monthValue, alarmInfo.netValueThreshold.initValue) / 0.8 * alarmInfo.netValueThreshold.percent;
            if (netValueU <= maxValue) {
                stringstream ss;
                string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);
                ss << "netValue小于等于" << maxValue << " 当前netValue(换成u)=" << netValueU;
                MsgCard msgCard;
                msgCard.name = name;
                msgCard.templateId = 3;
                msgCard.title = "NetValue";
                msgCard.object = "账户：" + name;
                msgCard.datetime = currentTimeStr;
                msgCard.content = ss.str();
                v.emplace_back(msgCard);
            }
        }
    }
    return v;
}

vector<MsgCard> ProductItem::GetFundingRateAlarmMsg() {
    int64_t currentTime = gettickcount();
    string currentTimeStr = CovertToUtcStr(currentTime * 1000, false);

    vector<MsgCard> v;
    string content = "";
    if (alarmInfo.fundingRateThreshold.alarm > 0) {
        for (auto iter = mPositionFundingRate.begin(); iter != mPositionFundingRate.end(); ++iter) {
            if ((iter->second.flag && iter->second.value > alarmInfo.fundingRateThreshold.alarm) || (!iter->second.flag && iter->second.value < -alarmInfo.fundingRateThreshold.alarm)) {   
                stringstream ss;
                ss << " Symbol: " << iter->first << " FundingRate超过阈值:" << alarmInfo.fundingRateThreshold.alarm << " 当前fundingRate=" << iter->second.value;
                content += ss.str();
            }
        }
    }

    if (content.length() > 0) {
        MsgCard msgCard;
        msgCard.name = name;
        msgCard.templateId = 2;
        msgCard.title = "FundingRate";
        msgCard.object = "账户：" + name;
        msgCard.datetime = currentTimeStr;
        msgCard.content = content;
        v.emplace_back(msgCard);
    }

    return v;
}

void ProductItem::UpdateAccountInfo() {
    for (size_t i = 0; i < vAccountId.size(); ++i) {
        int accountId = vAccountId[i];
	    BinanceAdapterItem* bItem = BinanceAdapterMgr::GetInstance().GetAdapterItem(accountId);
	    if (bItem) {
	        bItem->UpdateAccountInfo();
	    }

	    CoinbaseAdapterItem* cItem = CoinbaseAdapterMgr::GetInstance().GetAdapterItem(accountId);
	    if (cItem) {
	        cItem->UpdateAccountInfo();
	    }

	    GateioAdapterItem* gItem = GateioAdapterMgr::GetInstance().GetAdapterItem(accountId);
	    if (gItem) {
	        gItem->UpdateAccountInfo();
	    }

        BybitAdapterItem* bybItem = BybitAdapterMgr::GetInstance().GetAdapterItem(accountId);
	    if (bybItem) {
	        bybItem->UpdateAccountInfo();
	    }
    }
}

bool ProductItem::GetAdapterQueryStatus() {
    return adapterQuery;
}

igmonitor::RiskInfo& ProductItem::GetRiskInfo() {
    return riskInfo;
}
