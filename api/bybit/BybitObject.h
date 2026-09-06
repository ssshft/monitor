#pragma once

#include <string>

using namespace std;


namespace bybit {

    struct TotalAccountInfo {
        string accountType{""};
        double totalEquity{0.0};
        double accountIMRate{0.0};
        double totalMarginBalance{0.0};
        double totalInitialMargin{0.0};
        double totalAvailableBalance{0.0};
        double accountMMRate{0.0};
        double totalPerpUPL{0.0};
        double totalWalletBalance{0.0};
        double accountLTV{0.0};
        double totalMaintenanceMargin{0.0};
        
        string toString() {
            stringstream ss;
	        ss << "accountType:" << accountType << " totalEquity:" << totalEquity << " accountIMRate:" << accountIMRate << " totalMarginBalance:" << totalMarginBalance << " totalInitialMargin:" << totalInitialMargin << " totalAvailableBalance:" << totalAvailableBalance << " accountMMRate:" << accountMMRate << " totalPerpUPL:" << totalPerpUPL << " totalWalletBalance:" << totalWalletBalance << " accountLTV:" << accountLTV << " totalMaintenanceMargin:" << totalMaintenanceMargin;
	        return ss.str();
        }
    };

    struct Asset {
        string coin{""};
        double availableToBorrow{0.0};
        double bonus{0.0};
        double accruedInterest{0.0};
        double availableToWithdraw{0.0};
        double totalOrderIM{0.0};
        double equity{0.0};
        double totalPositionMM{0.0};
        double usdValue{0.0};
        double unrealisedPnl{0.0};
        double borrowAmount{0.0};
        double totalPositionIM{0.0};
        double walletBalance{0.0};
        double cumRealisedPnl{0.0};

        string toString() {
            stringstream ss;
	        ss << "coin:" << coin << " availableToBorrow:" << availableToBorrow << " bonus:" << bonus << " accruedInterest:" << accruedInterest << " availableToWithdraw:" << availableToWithdraw << " totalOrderIM:" << totalOrderIM << " equity:" << equity << " totalPositionMM:" << totalPositionMM << " usdValue:" << usdValue << " unrealisedPnl:" << unrealisedPnl << " borrowAmount:" << borrowAmount << " totalPositionIM:" << totalPositionIM << " walletBalance:" << walletBalance << " cumRealisedPnl:" << cumRealisedPnl;
	        return ss.str();     
        }
    };

    struct Position {
        string symbol{""};
        double size{0.0};
        string category{""};
        string side{""};
        double leverage{0.0};
        double avgPrice{0.0};
        double liqPrice{0.0};
        double takeProfit{0.0};
        double unrealisedPnl{0.0};
        double markPrice{0.0};
        double cumRealisedPnl{0.0};
        double positionMM{0.0};

        string toString() {
            stringstream ss;
	        ss << "symbol:" << symbol << " size:" << size << " side:" << side << " leverage:" << leverage << " avgPrice:" << avgPrice << " liqPrice:" << liqPrice << " takeProfit:" << takeProfit << " unrealisedPnl:" << unrealisedPnl << " markPrice:" << markPrice << " cumRealisedPnl:" << cumRealisedPnl << " positionMM:" << positionMM;
	        return ss.str();     
        }
    };

    struct Order {
        string orderId{""};
        string orderLinkId{""};
        string symbol{""};
        double price{0.0};
        double qty{0.0};
        string side{""};
        double avgPrice{0.0};
        double cumExecQty{0.0};
        double cumExecValue{0.0};
        double cumExecFee{0.0};
        string timeInForce{""};
        string orderType{""};
        int64_t createdTime{0};
        int64_t updatedTime{0};

        string toString() {
            stringstream ss;
            ss << "orderId:" << orderId << " orderLinkId:" << orderLinkId << " symbol:" << symbol << " price:" << price << " qty:" << qty << " side:" << side << " avgPrice:" << avgPrice << " cumExecQty:" << cumExecQty << " cumExecValue:" << cumExecValue << " cumExecFee:" << cumExecFee << " timeInForce:" << timeInForce << " orderType:" << orderType << " createdTime:" << createdTime << " updatedTime:" << updatedTime;
            return ss.str();     
        }
    };

}