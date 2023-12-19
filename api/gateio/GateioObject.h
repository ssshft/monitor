#pragma once

#include <string>

using namespace std;


namespace gateio {
    struct SpotAsset {
        string currency{""};
        double total{0.0};
        double available{0.0};
        double locked{0.0};
	    string toString() {
	        stringstream ss;
	        ss << "currency:" << currency << " total:" << total << " available:" << available << " locked:" << locked;
	        return ss.str();
	    }
    };

    struct FutureAsset {
        string currency{""};
        double total{0.0};
        double unrealisedPnl{0.0};
        double positionMargin{0.0};
        double orderMargin{0.0};
        double available{0.0};
        double positionInitialMargin{0.0};
        double point{0.0};
        double bonus{0.0};
        bool inDualMode{false};
	
	string toString() {
	    stringstream ss;
	    ss << "currency:" << currency << " total:" << total << " unrealisedPnl:" << unrealisedPnl << " positionMargin:" << positionMargin << " orderMargin:" << orderMargin << " available:" << available << " positionInitialMargin:" << positionInitialMargin << " point:" << point << " bonus:" << bonus << " inDualMode:" << inDualMode;
	    return ss.str();
	}
    };

    struct CrossMarginAsset {
        string currency{""};
        double available{0.0};
        double freeze{0.0};
        double borrowed{0.0};
        double interest{0.0};
        double negativeLiab{0.0};
        double futuresPosLiab{0.0};
        double equity{0.0};
        double totalFreeze{0.0};
        double totalLiab{0.0};

        string toString() {
            stringstream ss;
            ss << "currency:" << currency << " available:" << available << " freeze:" << freeze << " borrowed:" << borrowed << " interest:" << interest << " negativeLiab:" << negativeLiab << " futuresPosLiab:" << futuresPosLiab << " equity:" << equity << " totalFreeze:" << totalFreeze << " totalLiab:" << totalLiab;
            return ss.str();
        }
    };

    struct CrossMarginAccountTotal {
        double total;
        double borrowed;
        double interest;
        double risk;
        double totalInitialMargin;
        double totalMarginBalance;
        double totalMaintenanceMargin;
        double totalInitialMarginRate;
        double totalMaintenanceMarginRate;
        double totalAvailableMargin;
        double portfolioMarginTotal;
        double portfolioMarginTotalLiab;
        double portfolioMarginTotalEquity;
    };

    struct FuturePosition {
        string contract{""};
        string instrumentType{""};
        int64_t size{0};
        int leverage{0};
        int riskLimit{0};
        int leverageMax{0};
        double maintenanceRate{0.0};
        double value{0.0};
        double margin{0.0};
        double entryPrice{0.0};
        double liqPrice{0.0};
        double markPrice{0.0};
        double unrealisedPnl{0.0};
        double realisedPnl{0.0};
        double historyPnl{0.0};
        double lastClosePnl{0.0};
        double realisedPoint{0.0};
        double historyPoint{0.0};

	string toString() {
	    stringstream ss;
	    ss << "contract:" << contract << " instrumentType:" << instrumentType << " size:" << size << " leverage:" << leverage << " riskLimit:" << riskLimit << " leverageMax:" << leverageMax << " maintenanceRate:" << maintenanceRate << " value:" << value << " margin:" << margin << " entryPrice:" << entryPrice << " liqPrice:" << liqPrice << " markPrice:" << markPrice << " unrealisedPnl:" << unrealisedPnl << " realisedPnl:" << realisedPnl << " historyPnl:" << historyPnl << " lastClosePnl:" << lastClosePnl << " realisedPoint:" << realisedPoint << " historyPoint:" << historyPoint;
	    return ss.str();
	}
    };

    struct FutureOrder {
        string contract{""};
        int64_t createTime{0};
        int64_t finishTime{0};
        int size;
        int left;
        double price;
        double fillPrice;
        double mkfr;
        double tkfr;
        string status;
        string finishAs;

	string toString() {
	    stringstream ss;
	    ss << "contract:" << contract << " createTime:" << createTime << " finishTime:" << finishTime << " size:" << size << " left:" << left << " price:" << price << " fillPrice:" << fillPrice << " mkfr:" << mkfr << " tkfr:" << tkfr << " status:" << status << " finishAs:" << finishAs;
	    return ss.str();
	}
    };
}
