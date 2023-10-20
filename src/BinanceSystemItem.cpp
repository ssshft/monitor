#include "BinanceSystemItem.h"
#include "BasicInfoMgr.h"


BinanceSystemItem::BinanceSystemItem(int id, string n) {
    customerId = id;
    name = n;
    baseAsset = MonitorConfig::GetInstance().GetBaseAssetById(customerId);
}

BinanceSystemItem::~BinanceSystemItem() {
    vSpotAsset.clear();
    vUFutureAsset.clear();
    vCFutureAsset.clear();
    vUFuturePosition.clear();
    vCFuturePosition.clear();
}

void BinanceSystemItem::OnSubMessage(const web::json::value& content) {
    UpdateData(content);
    updateTime = GetCurrentTimeUs();
}

void BinanceSystemItem::UpdateData(const web::json::value& content) {
    if (content.has_field("exchange_id")) {
        ExchangeType exchengeId = (ExchangeType)content.at("exchange_id").as_integer();
        exchangeStr = ExchangeTypeEnum2StrMap[exchengeId];
    }
    
    if (content.has_field("acc_type")) {
        int accountType = content.at("acc_type").as_integer();

        vector<igsystem::SysAsset> vAsset;
        if (content.has_field("asset")) {
            const web::json::value& assetValue = content.at("asset");
            if (assetValue.is_array()) {
                auto& assetArr = assetValue.as_array();
                for (auto& a : assetArr) {
                    igsystem::SysAsset asset;
                    if (a.has_field("a")) {
                        asset.asset = a.at("a").as_string();
                    }
                    if (a.has_field("i")) {
                        asset.initAmount = a.at("i").as_double();
                    }
                    if (a.has_field("t")) {
                        asset.totalAmount = a.at("t").as_double();
                    }
                    if (a.has_field("f")) {
                        asset.frozenAmount = a.at("f").as_double();
                    }
                    if (a.has_field("m")) {
                        asset.marginAmount = a.at("m").as_double();
                    }
                    if (a.has_field("fee")) {
                        asset.feeAmount = a.at("fee").as_double();
                    }
                    if (a.has_field("c")) {
                        asset.closeAmount = a.at("c").as_double();
                    }
                    if (a.has_field("fa")) {
                        asset.floatAmount = a.at("fa").as_double();
                    }
                    if (a.has_field("pv")) {
                        asset.positionValue = a.at("pv").as_double();
                    }
                    if (a.has_field("pm")) {
                        asset.positionMargin = a.at("pm").as_double();
                    }
                    if (a.has_field("fm")) {
                        asset.frozenMargin = a.at("fm").as_double();
                    }
                    if (a.has_field("tf")) {
                        asset.transFrozenAmount = a.at("tf").as_double();
                    }
                    if (a.has_field("ti")) {
                        asset.totalInoutAmount = a.at("ti").as_double();
                    }
                    vAsset.emplace_back(asset);
                }
            }
        }

        vector<igsystem::SysPosition> vPosition;
        if (content.has_field("position")) {
            const web::json::value& positionValue = content.at("position");
            if (positionValue.is_array()) {
                auto& positionArr = positionValue.as_array();
                for (auto& p : positionArr) {
                    igsystem::SysPosition position;
                    if (p.has_field("s")) {
                        position.symbol = p.at("s").as_string();
                    }
                    if (p.has_field("lp")) {
                        position.longPosition = p.at("lp").as_double();
                    }
                    if (p.has_field("la")) {
                        position.longAvgPrice = p.at("la").as_double();
                    }
                    if (p.has_field("sp")) {
                        position.shortPosition = p.at("sp").as_double();
                    }
                    if (p.has_field("sa")) {
                        position.shortAvgPrice = p.at("sa").as_double();
                    }
                    if (p.has_field("f")) {
                        position.floatAmount = p.at("f").as_double();
                    }
                    if (p.has_field("c")) {
                        position.closeAmount = p.at("c").as_double();
                    }
                    if (p.has_field("pv")) {
                        position.positionValue = p.at("pv").as_double();
                    }
                    if (p.has_field("flv")) {
                        position.frozenLongPosition = p.at("flv").as_double();
                    }
                    if (p.has_field("flp")) {
                        position.frozenLongPrice = p.at("flp").as_double();
                    }
                    if (p.has_field("fsv")) {
                        position.frozenShortPosition = p.at("fsv").as_double();
                    }
                    if (p.has_field("fsp")) {
                        position.frozenShortPrice = p.at("fsp").as_double();
                    }
                    vPosition.emplace_back(position);
                }
            } 
        }

        if (accountType == AT_SPOT) {
            vSpotAsset = vAsset;
        } else if (accountType == AT_UFUTURES) {
            vUFutureAsset = vAsset;
            vUFuturePosition = vPosition;
        } else if (accountType == AT_CFUTURES) {
            vCFutureAsset = vAsset;
            vCFuturePosition = vPosition;
        }
    }
}

vector<igsystem::SysAsset>& BinanceSystemItem::GetSpotAsset() {
    return vSpotAsset;
}

vector<igsystem::SysAsset>& BinanceSystemItem::GetUFutureAsset() {
    return vUFutureAsset;
}

vector<igsystem::SysAsset>& BinanceSystemItem::GetCFutureAsset() {
    return vCFutureAsset;
}

vector<igsystem::SysPosition>& BinanceSystemItem::GetUFuturePosition() {
    return vUFuturePosition;
}

vector<igsystem::SysPosition>& BinanceSystemItem::GetCFuturePosition() {
    return vCFuturePosition;
}

set<string> BinanceSystemItem::GetInstrumentList() {
    set<string> s;
    if (exchangeStr.size() > 0) {
        if (baseAsset != "USDT") {
            string instrumentKey = exchangeStr + "|" + baseAsset + "USDT" + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = exchangeStr + "|" + baseAsset + "USDT" + "|SWAP";
            s.insert(instrumentKey);
        }

        for (size_t i = 0; i < vSpotAsset.size(); ++i) {
            string asset = vSpotAsset[i].asset;
            if (asset != baseAsset && asset != "USDT") {
                string instrumentKey = exchangeStr + "|" + asset + baseAsset + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = exchangeStr + "|" + asset + "USDT" + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = exchangeStr + "|" + asset + "USDT" + "|SWAP";
                s.insert(instrumentKey);
            }
        }

        for (size_t i = 0; i < vUFutureAsset.size(); ++i) {
            string asset = vUFutureAsset[i].asset;
            if (asset != baseAsset && asset != "USDT") {
                string instrumentKey = exchangeStr + "|" + asset + baseAsset + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = exchangeStr + "|" + asset + "USDT" + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = exchangeStr + "|" + asset + "USDT" + "|SWAP";
                s.insert(instrumentKey);
            }
        }

        for (size_t i = 0; i < vCFutureAsset.size(); ++i) {
            string asset = vCFutureAsset[i].asset;
            if (asset != baseAsset && asset != "USDT") {
                string instrumentKey = exchangeStr + "|" + asset + baseAsset + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = exchangeStr + "|" + asset + "USDT" + "|SPOT";
                s.insert(instrumentKey);
                instrumentKey = exchangeStr + "|" + asset + "USDT" + "|SWAP";
                s.insert(instrumentKey);
            }
        }

        for (size_t i = 0; i < vUFuturePosition.size(); ++i) {
            string instrumentKey = exchangeStr + "|" + vUFuturePosition[i].symbol + "|FUTURES";
            s.insert(instrumentKey);

            string key = BasicInfoMgr::GetInstance().GetSysIdByOriginId(instrumentKey);
            InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
            instrumentKey = exchangeStr + "|" + info.instLeft + baseAsset + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = exchangeStr + "|" + info.instLeft + "USDT" + "|SPOT";
            s.insert(instrumentKey);
            instrumentKey = exchangeStr + "|" + info.instLeft + "USDT" + "|SWAP";
            s.insert(instrumentKey);
        }

        for (size_t i = 0; i < vCFuturePosition.size(); ++i) {
            string instrumentKey = exchangeStr + "|" + vCFuturePosition[i].symbol + "|FUTURES";
            s.insert(instrumentKey);
        }
    }
    
    return s;
}

string BinanceSystemItem::GetExchangeStr() {
    return exchangeStr;
}

int64_t BinanceSystemItem::GetUpdateTime() {
    return updateTime;
}