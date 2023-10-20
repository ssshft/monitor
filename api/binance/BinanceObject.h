#pragma once

#include <string>
using namespace std;

namespace binance {
    struct SpotAsset {
        string assetType{""};
        double free{0.0};
        double locked{0.0};

        string toString() {
            stringstream ss;
            ss << "assetType:" << assetType << " free:" << free << " locked:" << locked;
            return ss.str();
        }
    };

    struct SpotTrade {
        string symbol{""};
        int64_t id{0};
        int64_t orderId{0};
        int64_t orderListId{0};
        double price{0.0};
        double qty{0.0};
        double quoteQty{0.0};
        double commission{0.0};
        string commissionAsset{""};
        int64_t time{0};
        bool isBuyer{false};
        bool isMaker{false};
        bool isBestMatch{false};
    };

    struct UFutureAsset {
        string assetType{""};
        double walletBalance{0.0};
        double unrealizedProfit{0.0};
        double marginBalance{0.0};
        double maintMargin{0.0};
        double initialMargin{0.0};
        double positionInitialMargin{0.0};
        double openOrderInitialMargin{0.0};
        double crossWalletBalance{0.0};
        double crossUnPnl{0.0};
        double availableBalance{0.0};
        double maxWithdrawAmount{0.0};
        bool marginAvailable{0.0};
        int64_t updateTime{0};

        string toString() {
            stringstream ss;
            ss << "assetType:" << assetType << " walletBalance:" << walletBalance << " unrealizedProfit:" << unrealizedProfit << " marginBalance:" << marginBalance << " maintMargin:" << maintMargin << " initialMargin:" << initialMargin << " positionInitialMargin:" << positionInitialMargin << " openOrderInitialMargin:" << openOrderInitialMargin << " crossWalletBalance:" << crossWalletBalance << " crossUnPnl:" << crossUnPnl << " availableBalance:" << availableBalance << " maxWithdrawAmount:" << maxWithdrawAmount << " marginAvailable:" << marginAvailable << " updateTime:" << updateTime;
            return ss.str();
        }
    };

    struct UFuturePosition {
        string symbol{""};
        double initialMargin{0.0};
        double maintMargin{0.0};
        double unrealizedProfit{0.0};
        double positionInitialMargin{0.0};
        double openOrderInitialMargin{0.0};
        double leverage{0.0};
        bool isolated{0.0};
        double entryPrice{0.0};
        double maxNotional{0.0};
        double bidNotional{0.0};
        double askNotional{0.0};
        string positionSide{""};
        double positionAmt{0.0};
        int64_t updateTime{0};

        string toString() {
            stringstream ss;
            ss << "symbol:" << symbol << " initialMargin:" << initialMargin << " maintMargin:" << maintMargin << " unrealizedProfit:" << unrealizedProfit << " positionInitialMargin:" << positionInitialMargin << " openOrderInitialMargin:" << openOrderInitialMargin << " leverage:" << leverage << " isolated:" << isolated << " entryPrice:" << entryPrice << " maxNotional:" << maxNotional << " bidNotional:" << bidNotional << " askNotional:" << askNotional << " positionAmt:" << positionAmt << " positionSide:" << positionSide << " updateTime:" << updateTime;
            return ss.str();
        }

    };

    struct UFutureTrade {
        string symbol{""};
        int64_t id{0};
        int64_t orderId{0};
        double price{0.0};
        double qty{0.0};
        double quoteQty{0.0};
        double realizedPnl{0.0};
        string side{""};
        string positionSide{""};
        double commission{0.0};
        string commissionAsset{""};
        int64_t time{0};
        bool isBuyer{false};
        bool isMaker{false};
    };

    struct CFutureAsset {
        string assetType{""};
        double walletBalance{0.0};
        double unrealizedProfit{0.0};
        double marginBalance{0.0};
        double maintMargin{0.0};
        double initialMargin{0.0};
        double positionInitialMargin{0.0};
        double openOrderInitialMargin{0.0};
        double crossWalletBalance{0.0};
        double crossUnPnl{0.0};
        double availableBalance{0.0};
        double maxWithdrawAmount{0.0};

        string toString() {
            stringstream ss;
            ss << "assetType:" << assetType << " walletBalance:" << walletBalance << " unrealizedProfit:" << unrealizedProfit << " marginBalance:" << marginBalance << " maintMargin:" << maintMargin << " initialMargin:" << initialMargin << " positionInitialMargin:" << positionInitialMargin << " openOrderInitialMargin:" << openOrderInitialMargin << " crossWalletBalance:" << crossWalletBalance << " crossUnPnl:" << crossUnPnl << " availableBalance:" << availableBalance << " maxWithdrawAmount:" << maxWithdrawAmount;
            return ss.str();
        }
    };

    struct CFuturePosition {
        string symbol{""};
        double initialMargin{0.0};
        double maintMargin{0.0};
        double unrealizedProfit{0.0};
        double positionInitialMargin{0.0};
        double openOrderInitialMargin{0.0};
        double leverage{0.0};
        bool isolated{false};
        double entryPrice{0.0};
        double maxQty{0.0};
        string positionSide{""};
        double positionAmt{0.0};
        int64_t updateTime{0};

        string toString() {
            stringstream ss;
            ss << "symbol:" << symbol << " initialMargin:" << initialMargin << " maintMargin:" << maintMargin << " unrealizedProfit:" << unrealizedProfit << " positionInitialMargin:" << positionInitialMargin << " openOrderInitialMargin:" << openOrderInitialMargin << " leverage:" << leverage << " isolated:" << isolated << " entryPrice:" << entryPrice << " maxQty:" << maxQty << " positionAmt:" << positionAmt << " positionSide:" << positionSide << " updateTime:" << updateTime;
            return ss.str();
        }
    };

    struct CFutureTrade {
        string symbol{""};
        int64_t id{0};
        int64_t orderId{0};
        string pair{""};
        string side{""};
        double price{0.0};
        double qty{0.0};
        double realizedPnl{0.0};
        string marginAsset{""};
        double baseQty{0.0};
        double commission{0.0};
        string commissionAsset{""};
        int64_t time{0};
        string positionSide{""};
        bool isBuyer{false};
        bool isMaker{false};
    };

    struct PositionRisk {
        string symbol{""};
        double liquidationPrice{0.0};
        string positionSide{""};
    };

    struct SpotOpenOrder {
        string symbol{""};
        int64_t orderId{0};
        string clientOrderId{""};
        double price{0.0};
        double origQty{0.0};      // 原始委托数量
        double executedQty{0.0};  // 成交量  
        double cumQuote{0.0};
        string side{""};
        int64_t time{0};
    };

    struct FutureOpenOrder {
        string symbol{""};
        int64_t orderId{0};
        string clientOrderId{""};
        double price{0.0};
        double origQty{0.0};      // 原始委托数量
        double executedQty{0.0};  // 成交量  
        double avgPrice{0.0};
        double cumQuote{0.0};
        string side{""};
        string positionSide{""};
        int64_t time{0};
	
        string toString() {
            stringstream ss;
            ss << "symbol:" << symbol << " orderId:" << orderId << " clientOrderId:" << clientOrderId << " price:" << price << " origQty:" << origQty << " executedQty:" << executedQty << " avgPrice:" << avgPrice << " cumQuote:" << cumQuote << " side:" << side << " positionSide:" << positionSide << " time:" << time;
            return ss.str();
        }
    };

    struct SavingAsset {
        string asset{""};
        double amount{0.0};
        double amountInBTC{0.0};
        double amountInUSDT{0.0};

	    string toString() {
	        stringstream ss;
	        ss << "asset:" << asset << " amount:" << amount << " amountInBTC:" << amountInBTC << " amountInUSDT:" << amountInUSDT;
	        return ss.str();
	    }
    };

    struct SavingInterest {
        string asset{""};
        double interest{0.0};
        string lendingType{""};
        string productName{""};
        int64_t time{0};
    };

    struct SavingPurchase {
        string asset{""};
        double amount{0.0};
        string lendingType{""};
        string productName{""};
        int lot{0};
        int purchaseId{0};
        string status{""};
        int64_t createTime{0};
    };

    struct SavingRedemption {
        string asset{""};
        double amount{0.0};
        double principal{0.0};
        string projectId{""};
        string projectName{""};
        string status{""};
        int64_t createTime{0};
    };

    struct TotalMarginAsset {
        double marginLevel{0.0};
        double totalAssetOfBtc{0.0};
        double totalLiabilityOfBtc{0.0};
        double totalNetAssetOfBtc{0.0};

        string toString() {
	        stringstream ss;
	        ss << "marginLevel:" << marginLevel << " totalAssetOfBtc:" << totalAssetOfBtc << " totalLiabilityOfBtc:" << totalLiabilityOfBtc << " totalNetAssetOfBtc:" << totalNetAssetOfBtc;
	        return ss.str();
	    }
    };

    struct MarginAsset {
        string asset{""};
        double borrowed{0.0};
        double free{0.0};
        double interest{0.0};
        double locked{0.0};
        double netAsset{0.0};

        string toString() {
	        stringstream ss;
	        ss << "asset:" << asset << " borrowed:" << borrowed << " free:" << free << " interest:" << interest << " locked:" << locked << " netAsset:" << netAsset;
	        return ss.str();
	    }
    };

    struct LiquidityInfo {
        int poolId;
        string poolName;
        int64_t updateTime;
        map<string, double> mLiquidityAsset;
        int64_t shareAmount;
        double sharePercentage;
        map<string, double> mShareAsset;
    };

    struct UnclaimedRewards {
        map<string, double> mTotalUnclaimedRewards;
    };

    struct StakingPosition {
        string positionId;
        string projectId;
        string asset;
        double amount;
        int64_t purchaseTime;
        int duration;
        int accrualDays;
        string rewardAsset;
        double apy;
        double rewardAmt;
        string extraRewardAsset;
        double extraRewardApy;
        double estExtraRewardAmt;
        double nextInterestPay;
        int64_t nextInterestPayDate;
        int payInterestPeriod;
        double redeemAmountEarly;
        int64_t interestEndDate;
        int64_t deliverDate;
        int redeemPeriod;
        double redeemingAmt;
        int64_t partialAmtDeliverDate;
        double canRedeemEarly;
        double renewable;
        string type;
        string status;
    };

    struct LoanBorrow {
        string loanCoin{""};
        double loanAmount{0};
        string collateralCoin{""};
        double collateralAmount{0};

        string toString() {
            stringstream ss;
            ss << "loanCoin:" << loanCoin << " loanAmount:" << loanAmount << " collateralCoin:" << collateralCoin << " collateralAmount:" << collateralAmount;
            return ss.str();
        }
    };

    // unify account
    struct UnifyAsset {
        string asset{""};
        double totalWalletBalance{0.0};
        double crossMarginAsset{0.0};
        double crossMarginBorrowed{0.0};
        double crossMarginFree{0.0};
        double crossMarginInterest{0.0};
        double crossMarginLocked{0.0};
        double crossMarginNetAsset{0.0};
        double umWalletBalance{0.0};
        double umUnrealizedPnl{0.0};
        double cmWalletBalance{0.0};
        double cmUnrealizedPnl{0.0};
        int64_t updateTime;

        string toString() {
            stringstream ss;
            ss << "asset:" << asset << " totalWalletBalance:" << totalWalletBalance << " crossMarginAsset:" << crossMarginAsset << " crossMarginBorrowed:" << crossMarginBorrowed << " crossMarginFree:" << crossMarginFree << " crossMarginInterest:" << crossMarginInterest << " crossMarginLocked:" << crossMarginLocked << " crossMarginNetAsset:" << crossMarginNetAsset << " umWalletBalance:" << umWalletBalance << " umUnrealizedPnl:" << umUnrealizedPnl << " cmWalletBalance:" << cmWalletBalance << " cmUnrealizedPnl:" << cmUnrealizedPnl << " updateTime:" << updateTime;
            return ss.str();
        }
    };

    struct UnifyAccount {
        double uniMMR{0.0};
        double accountEquity{0.0};
        double actualEquity{0.0};
        double accountInitialMargin{0.0};
        double accountMaintMargin{0.0};
        string accountStatus{""};
        double virtualMaxWithdrawAmount{0.0};
        double totalAvailableBalance{0.0};
        double totalMarginOpenLoss{0.0};
        int64_t updateTime;

        string toString() {
            stringstream ss;
            ss << "uniMMR:" << uniMMR << " accountEquity:" << accountEquity << " actualEquity:" << actualEquity << " accountInitialMargin:" << accountInitialMargin << " accountMaintMargin:" << accountMaintMargin << " accountStatus:" << accountStatus << " virtualMaxWithdrawAmount:" << virtualMaxWithdrawAmount << " totalAvailableBalance:" << totalAvailableBalance << " totalMarginOpenLoss:" << totalMarginOpenLoss << " updateTime:" << updateTime;
            return ss.str();
        }
    };

    struct UnifyPosition {
        string symbol{""};
        double entryPrice{0.0};
        double leverage{0.0};
        double markPrice{0.0};
        double maxNotionalValue{0.0};
        double positionAmt{0.0};
        double notional{0.0};
        double unRealizedProfit{0.0};
        string positionSide;
        int64_t updateTime;

        string toString() {
            stringstream ss;
            ss << "symbol:" << symbol << " entryPrice:" << entryPrice << " leverage:" << leverage << " markPrice:" << markPrice << " maxNotionalValue:" << maxNotionalValue << " positionAmt:" << positionAmt << " notional:" << notional << " unRealizedProfit:" << unRealizedProfit << " positionSide:" << positionSide << " updateTime:" << updateTime;
            return ss.str();
        }
    };

    struct UnifyOpenOrder {
        string symbol{""};
        int64_t orderId{0};
        string clientOrderId{""};
        double price{0.0};
        double origQty{0.0};      // 原始委托数量
        double executedQty{0.0};  // 成交量  
        double avgPrice{0.0};
        double cumQuote{0.0};
        string side{""};
        string positionSide{""};
        int64_t time{0};
	
        string toString() {
            stringstream ss;
            ss << "symbol:" << symbol << " orderId:" << orderId << " clientOrderId:" << clientOrderId << " price:" << price << " origQty:" << origQty << " executedQty:" << executedQty << " avgPrice:" << avgPrice << " cumQuote:" << cumQuote << " side:" << side << " positionSide:" << positionSide << " time:" << time;
            return ss.str();
        }
    };    
}


