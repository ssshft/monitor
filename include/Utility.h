#pragma once

#include "NanoLogLite/Log.h"
#include "NanoLogLite/NanoLog.h"
#include <openssl/hmac.h>
#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <chrono>
#include <string>
#include <vector>
#include <set>
#include <boost/algorithm/string/case_conv.hpp>

using namespace std;
using namespace std::chrono;

extern std::mutex mut;
extern std::mutex mutRiskParameter;

enum ExchangeType {
	ET_MIN = 0,
    ET_OKX = 1,
	ET_BINANCE,
	ET_FTX,
	ET_XT,
	ET_BYBIT,
	ET_COINBASE,
	ET_HUOBI,
	ET_GATEIO,
	ET_ZB,
	ET_MEXC,
	ET_DERIBIT,
	ET_KUCOIN,	
	ET_MAX
};

enum OrderStatus {
	OST_MIN = 0,
	OST_PENDING,
	OST_NEW,
	OST_PARTFILLED,
	OST_CANCELING,
	OST_FAILED,
	OST_FINISHED,
	OST_FILLED,
	OST_CANCELED,
	OST_REJECTED,
	OST_EXPIRED
};

enum AccountType {
	AT_MIN,
	AT_FUND,
    AT_SPOT,
    AT_UFUTURES,
    AT_CFUTURES,
    AT_MARGIN,
    AT_SWAP,
	AT_DELIVERY,
	AT_PERPETUAL,
	AT_MAX
};

enum InstrumentType {
	MARGIN,
    SWAP,
    FUTURES,
    SPOT
};

enum MarketType {
	TRADES,
    FUNDING_RATE,
    DEPTH5,
    DEPTH10,
    DEPTH20,
    KLINE_1m,
    KLINE_3m,
    KLINE_5m,
    KLINE_15m,
    KLINE_30m,
    KLINE_1h,
    KLINE_2h,
    KLINE_4h,
    KLINE_6h,
    KLINE_12h,
    KLINE_1d,
    KLINE_3d,
    KLINE_1w,
    KLINE_1M
};

enum CommandType {
	INITIALIZATION,
	NEW_ORDER, 
	NEW_CANCEL_ORDER,
	TRANSFER,
	QUERY_ORDER, 
	ORDER_REPORT,
	CANCEL_ORDER_REPORT,
	ACCOUNT_UPDATE_REPORT,
	TRANSFER_REPORT,
	QUERY_ORDER_REPORT,
	QUERY_FUND_REPORT,
	QUERY_POSITION_REPORT,
	BALANCE_UPDATE_REPORT,
	ASSET_AND_POSITION_UPDATE_REPORT,
	ERROR_REPORT = 50
};

enum AssetReportType {
	ASSET_ONLY_CHANGE,
	ASSET_ALL
};

static unordered_map<ExchangeType, string> ExchangeTypeEnum2StrMap {
    { ET_OKX, "OKX"},
    { ET_BINANCE, "BINANCE"},
    { ET_FTX, "FTX"},
    { ET_XT, "XT"},
	{ ET_BYBIT, "BYBIT"},
    { ET_COINBASE, "COINBASE"},
    { ET_HUOBI,"HUOBI"},
    { ET_GATEIO, "GATEIO"},
    { ET_ZB, "ZB"},
    { ET_MEXC, "MEXC"},
    { ET_DERIBIT, "DERIBIT"},
    { ET_KUCOIN, "KUCOIN"}
};

static unordered_map<OrderStatus, string> OrderStatusEnum2StrMap {
    {OST_PENDING, "PENDING"},
	{OST_NEW, "NEW"},
	{OST_PARTFILLED, "PARTFILLED"},
	{OST_CANCELING, "CANCELING"},
	{OST_FAILED, "FAILED"},
	{OST_FINISHED, "FINISHED"},
	{OST_FILLED, "FILLED"},
	{OST_CANCELED, "CANCELED"},
	{OST_REJECTED, "REJECTED"},
	{OST_EXPIRED, "EXPIRED"},
};


namespace igsystem {
	struct SysAsset {
		string asset{""};
		double initAmount{0.0};
		double totalAmount{0.0};
		double frozenAmount{0.0};
		double marginAmount{0.0};
		double feeAmount{0.0};
		double closeAmount{0.0};
		double floatAmount{0.0};
		double positionValue{0.0};
		double positionMargin{0.0};
		double frozenMargin{0.0};
		double transFrozenAmount{0.0};
		double totalInoutAmount{0.0};
	};

	struct SysPosition {
		string symbol{""};
		double longPosition{0.0};
		double longAvgPrice{0.0};
		double shortPosition{0.0};
		double shortAvgPrice{0.0};
		double floatAmount{0.0};
		double closeAmount{0.0};
		double positionValue{0.0};
		double frozenLongPosition{0.0};
		double frozenLongPrice{0.0};
		double frozenShortPosition{0.0};
		double frozenShortPrice{0.0};
	};
};


struct AccountInfo {
	string accountName;
	int accountId;
	ExchangeType exchangeType;
	string apiKey;
	string secretKey;
	string passphrase;
	string userId;
	string restUrl;
	string wssUrl;
	string baseAsset;
	string apiPermission;
	int hedge;
	int unified; // 是否时统一账户
};

struct ProductInfo {
	int productId;
	string productName;
	string baseAsset;
	vector<int> vAccountId;
};

struct AccountMonitorInfo {
	string addr;
	int port;
	string password;
	string channels;
};

struct SubData {
    string channel{""};
    string content{""};
};

struct Depth {
	string exchangeType{""};
	string instrumentType{""};
	string marketType{""};
	string instrumentId{""};
	vector<double> askP;
	vector<double> askV;
	vector<double> bidP;
	vector<double> bidV;
	int64_t ts{0};
	int64_t tsNet{0};
	int64_t tsParse{0};
	int64_t tsLocal{0};
};

struct Kline {
	string exchangeType{""};
	string instrumentType{""};
	string marketType{""};
	string instrumentId{""};
	int64_t barTime{0};
	double highPrice{0.0};
	double lowPrice{0.0};
	double openPrice{0.0};
	double closePrice{0.0};
	double avgPrice{0.0};
	double totalVolume{0.0};
	double totalAmount{0.0};
	double takerLongVolume{0.0};
	double takerLongAmount{0.0};
	double takerShortVolume{0.0};
	double takerShortAmount{0.0};
	int numOfTrade{0};
	string isFinished{""};
	int64_t ts{0};
	int64_t tsNet{0};
	int64_t tsParse{0};
	int64_t tsLocal{0};
};

struct Rate {
	string exchangeType{""};
	string instrumentType{""};
	string marketType{""};
	string instrumentId{""};
	double fundingRate{0.0};
	double nextFundingRate{0.0};
	int64_t fundingTime{0};
	int64_t ts{0};
	int64_t tsNet{0};
	int64_t tsParse{0};
};

struct Trade {
	string exchangeType{""};
	string instrumentType{""};
	string marketType{""};
	string instrumentId{""};
	int64_t tradeId{0};
	double px{0.0};
	double size{0.0};
	string side{""};
	int64_t ts{0};
	int64_t tsNet{0};
	int64_t tsParse{0};
};

namespace igmonitor {
	struct Asset {
		string asset; 
    	double realLeverageRatioS;
		double realLeverageRatioD;
    	double positionValueS;
		double positionValueD;
    	double transferAmount;
		double transferFrozenAmount;
    	double netAmountS;
		double netAmountD;
    	double availableAmountS;
		double availableAmountD;
    	double totalAmountS;
    	double totalAmountD;
    	double frozenAmountS;
    	double frozenAmountD;
    	double floatAmountS;
    	double floatAmountD;
    	double marginAmountS;
    	double marginAmountD;
    	double frozenMarginAmountS;
    	double frozenMarginAmountD;
		double underwayOrderValueS;
		double underwayOrderValueD;
    	double initialAmount;

		Asset() {
			asset = ""; 
    		realLeverageRatioS = 0.0;
			realLeverageRatioD = 0.0;
    		positionValueS = 0.0;
			positionValueD = 0.0;
    		transferAmount = 0.0;
			transferFrozenAmount = 0.0;
    		netAmountS = 0.0;
			netAmountD = 0.0;
    		availableAmountS = 0.0;
			availableAmountD = 0.0;
    		totalAmountS = 0.0;
    		totalAmountD = 0.0;
    		frozenAmountS = 0.0;
    		frozenAmountD = 0.0;
    		floatAmountS = 0.0;
    		floatAmountD = 0.0;
    		marginAmountS = 0.0;
    		marginAmountD = 0.0;
    		frozenMarginAmountS = 0.0;
    		frozenMarginAmountD = 0.0;
			underwayOrderValueS = 0.0;
			underwayOrderValueD = 0.0;
    		initialAmount = 0.0;
		}
	};

	struct Position {
		string key;
		string symbol;
		double netPositionS;
		double netPositionD;
		double netAvgPriceS;
		double netAvgPriceD;
		double floatAmountS;
		double floatAmountD;
		double underwayNetPositionS;
		double underwayNetPositionD;
		double underwayAbsPositionS;
		double underwayAbsPositioD;
    	double multiple;
    	double price;
		double liquidationPrice;

		Position() {
			key = " ";
			symbol = " ";
			netPositionS = 0.0;
			netPositionD = 0.0;
			netAvgPriceS = 0.0;
			netAvgPriceD = 0.0;
			floatAmountS = 0.0;
			floatAmountD = 0.0;
			underwayNetPositionS = 0.0;
			underwayNetPositionD = 0.0;
			underwayAbsPositionS = 0.0;
			underwayAbsPositioD = 0.0;
    		multiple = 0.0;
    		price = 0.0;
			liquidationPrice = 0.0;
		}
	};

	struct Exposure {
		string asset;
		double initialAmount;
    	double initialValue;
    	double exposureAmountS;
		double exposureAmountD;
    	double exposureValueS;
		double exposureValueD;
    	double deltaAmountS;
		double deltaAmountD;
    	double deltaValueS;
		double deltaValueD;
		Exposure() {
			asset = "";
			initialAmount = 0.0;
    		initialValue = 0.0;
    		exposureAmountS = 0.0;
			exposureAmountD = 0.0;
    		exposureValueS = 0.0;
			exposureValueD = 0.0;
    		deltaAmountS = 0.0;
			deltaAmountD = 0.0;
    		deltaValueS = 0.0;
			deltaValueD = 0.0;
		}
	};

	struct RiskInfo {
		int accountId{0};
		string name{""};
    	int64_t tsLocal{0};
		int64_t sUpdateTime{0};
		int64_t dUpdateTime{0};
		string baseAsset{""};
    	double yNetValue{0.0};
    	double netValueS{0.0};
		double netValueD{0.0};
    	double netValueGrowthRateS{0.0};
		double netValueGrowthRateD{0.0};
    	double maxValueToLoanRatioS{0.0};
		double maxValueToLoanRatioD{0.0};
    	double maxRealLeverageS{0.0};
		double maxRealLeverageD{0.0};
    	double riskExposureS{0.0};
		double riskExposureD{0.0};
    	double underwayOrderValueS{0.0};
		double underwayOrderValueD{0.0};
		int requestPerMinute{0};
		double verifyRiskS{0.0};
		double verifyRiskD{0.0};
		double utilizationS{0.0};
		double utilizationD{0.0};
		vector<int> vStrategyId;
		vector<int> vTraderId;

		string toString() {
			stringstream ss;
			ss << "accountId:" << accountId << " name:" << name << " baseAsset:" << baseAsset << " netValueD:" << netValueD << " netValueS:" << netValueS << " maxRealLeverageD:" << maxRealLeverageD << " maxRealLeverageS:" << maxRealLeverageS << " riskExposureD:" << riskExposureD << " riskExposureS:" << riskExposureS << " underwayOrderValueD:" << underwayOrderValueD << " underwayOrderValueS:" << underwayOrderValueS;
			return ss.str();
		}
	};

	struct SavAsset {
    	string asset{""};
    	double amountD{0.0};
    	double amountInBTCD{0.0};
    	double amountInUSDTD{0.0};
	}; 

	struct OpenOrder {
		string symbol{""};
		double openQty{0.0};
	};

	struct Order {
		string symbol{""};
		string instType{""};
		double volume{0.0};
		double price{-1};
		double filledVolume{0.0};
		double avgPrice{-1};
		string side{""};
		string status{""};
		string category{""};
		int64_t updateTime{0};  // 毫秒
		int64_t createTime{0};

        string toString() {
            stringstream ss;
            ss << "symbol:" << symbol << " instType:" << instType << " volume:" << volume << " price:" << price << " filledVolume:" << filledVolume << " avgPrice:" << avgPrice << " side:" << side << " status:" << status << " category:" << category << " updateTime:" << updateTime << " createTime:" << createTime;
            return ss.str();
        }
	};

	struct TotalMarAsset {
    	double marginLevel;
    	double totalAssetOfBtc;
    	double totalLiabilityOfBtc;
    	double totalNetAssetOfBtc;
		TotalMarAsset() {
			marginLevel = 0.0;
    		totalAssetOfBtc = 0.0;
    		totalLiabilityOfBtc = 0.0;
    		totalNetAssetOfBtc = 0.0;
		}
	};

	struct MarAsset {
		string asset{""};
		double borrowed{0.0};
		double free{0.0};
		double interest{0.0};
		double locked{0.0};
		double netAsset{0.0};
	};

	struct PositionFundingRate {
		string symbol{""};
		double netPosition;
		bool flag{false};
		double value{0.0};
	};

	struct LoBo {
        string loanCoin{""};
        double loanAmount{0};
        string collateralCoin{""};
        double collateralAmount{0};
    };

	struct MarketInfo {
		string instrumentKey{""};
		double amount{0};
		double twentyHourAmount{0};
		double openInterest{0};
		double liquid{0};
		double concentration{0};
	};
};

struct InstrumentInfo {
	string exchangeType{""};
	string instrumentType{""};
	string instrumentId{""};
	string originInstrumentId{""};
	double multiple{0.0};
	double multipleVolume{0};
	int calculateType{0};
	string deliveryDate{""};
	int lever{0};
	double tickSize{0.0};
	double lotSize{0.0};
	double minSize{0.0};
	string instLeft{""};
	string instRight{""};
	string margin{""};
    string left{""};
    string right{""};
};

struct DbOrder {
	int cmdId{0};
    int accountId{0};
    int customerId{0};
	int accountType{0};
    int64_t orderId{0};
    string clientOrderId{""};
    string symbol{""};
    int exchangeId{0};
    string side{""};
    double price{0.0};
	double quantity{0.0};
    double cumuFilledQuantity{0.0};
    int status{0};
    string statusMsg{""};
	double avgPrice{0.0};
	double commissionAmount{0.0};
    string commissionAsset{""};
    double lastExecutedQuantity{0.0};
    double lastExecutedPrice{0.0};
	string eventTime{""};
    int errType{0};
	int errCode{0};
	string errMsg{""};
	int64_t tsLocal{0};

	string toStr() {
        stringstream ss;
        ss << "cmdId:" << cmdId << " accountId:" << accountId << " customerId:" << customerId << " orderId:" << orderId << " clientOrderId:" << clientOrderId << " symbol:" << symbol << " exchangeId:" << exchangeId << " side:" << side << " price:" << price << " quantity:" << quantity << " cumuFilledQuantity:" << cumuFilledQuantity << " status:" << status << " statusMsg:" << statusMsg << " avgPrice:" << avgPrice << " commissionAmount:" << commissionAmount << " commissionAsset:" << commissionAsset << " lastExecutedQuantity:" << lastExecutedQuantity << " lastExecutedPrice:" << lastExecutedPrice << " eventTime:" << eventTime << " errType:" << errType << " errCode:" << errCode << " errMsg:" << errMsg;
        return ss.str();
    }
};

struct SymbolMarketInfo {
	string symbol{""};
	double volume{0};
	double openInterest{0};
};

struct LeverageThreshold {
	double warning{-1};
	double alarm{-1};
};

struct RiskExposureThreshold {
	double warning{-1};
	double alarm{-1};
	double uwarning{-1};
	double ualarm{-1};
	double totalExposureWarning{-1};
	double totalExposureAlarm{-1};
	set<string> sNoWarningAsset;
};

struct UnderwayOrderValueThreshold {
	double warning{-1};
	double alarm{-1};
};

struct NetValueThreshold {
	double monthValue{-1};
	double initValue{-1};
	double percent{1};
};

struct FundingRateThreshold {
	double alarm{-1};
};

struct LiquidationPriceThreshold {
	double warning{-1};
	double alarm{-1};
};

struct MarginRateThreshold {
	double initialMarginRateWarning{0.0};
	double initialMarginRateAlarm{0.0};
	double unifyMaintenanceMarginRateWarning{0.0};
	double unifyMaintenanceMarginRateAlarm{0.0};
};

struct AlarmInfo {
	LeverageThreshold leverageThreshold;
	RiskExposureThreshold riskExposureThreshold;
	UnderwayOrderValueThreshold underwayOrderValueThreshold;
	NetValueThreshold netValueThreshold;
	FundingRateThreshold fundingRateThreshold;
	LiquidationPriceThreshold liquidationPriceThreshold;
	MarginRateThreshold marginRateThreshold;
};

struct OrderValueThreshold {
	double orderNetValuePercent{-1};
    double orderUsdtValueDown{-1};
    double orderUsdtValueUp{-1};
};

struct AccountVoiceCall {
	string groupId{""};
	vector<string> vUserId;
	vector<string> vUserIdNetValue;
	vector<string> vUserIdLiquidationPrice;
};

struct MdPriceVoiceCall {
	string symbol{""};
	double price{0.0};
	int mode{0};
	vector<string> vUserId;
};

struct ReceiveInfo {
	string id{""};
	string type{""};
};

struct ReceiveGroupInfo {
	string code{""};
	string importance{""};
};

struct MsgCard {
	int accountId{0};
	int templateId{1};
	string name{""};
	string title{""};
	string object{""};
	string datetime{""};
	string content{""};
};

inline int64_t GetCurrentTimeUs() { // 微妙
	return high_resolution_clock::now().time_since_epoch().count() / 1000;
}

inline int64_t gettickcount() {  // 毫秒
    return GetCurrentTimeUs() / 1000; 
}

inline tm GetCurrentTimeTm() {
	int64_t tUs = GetCurrentTimeUs();
	time_t ts = tUs / 1000000;
	tm tmT = *gmtime(&ts);
	return tmT;
}

inline tm CovertToUtcTm(int64_t tUs) {
	time_t ts = tUs / 1000000;
	tm tmT = *gmtime(&ts);
	return tmT;
}

inline string CovertToUtcStr(int64_t tUs, bool hasUs = true) {
	if (tUs <= 0) {
		return "0";
	}
	time_t ts = tUs / 1000000;
	long us = tUs % 1000000;
	struct tm tmT = *gmtime(&ts);
	char s[64];
	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tmT);
	stringstream ss;
	if (hasUs) {
		ss << s << "." << us;
	} else {
		ss << s;
	}
	
	return ss.str();
}

inline string CovertToUtcDateStr(int64_t tUs) {
	if (tUs <= 0) {
		return "0";
	}
	time_t ts = tUs / 1000000;
	struct tm tmT = *gmtime(&ts);
	char s[64];
	strftime(s, sizeof(s), "%Y-%m-%d", &tmT);	
	return string(s);
}

inline string GetTimestamp() {
	char timestamp[32]{0};
	time_t t;
	time(&t);
	struct tm* ptm = gmtime(&t);
	strftime(timestamp, 32, "%FT%T.123Z", ptm);
	return timestamp;
}

inline void InitLog(int logLevel) {
	time_t now;
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now);
    char logName[128];

    sprintf(logName,"logriskmonitor_%4d%02d%02d.log",
            tm_now->tm_year + 1900,
            tm_now->tm_mon + 1,
            tm_now->tm_mday);

    NanoLog::setLogFile(logName);
    NanoLog::setLogLevel((NanoLog::LogLevel)logLevel);
}

inline bool SplitString(string& strSource, char* cstrSegmentStopC, vector<string>& strArray) {
	if (cstrSegmentStopC == NULL) {
		strArray.push_back(strSource);
		return true;
	}

	int iLength = strSource.size();
	int iFirst = 0, iLast = -1;
	string strKey, strValue;
	string strSubsource = strSource;
	while (iLast < iLength - 1) {
		iLast = strSource.find(cstrSegmentStopC, iFirst);
		iLast = iLast == string::npos ? iLength : iLast;
		strSubsource = strSource.substr(iFirst, iLast - iFirst);
		strArray.push_back(strSubsource);
		iFirst = iLast + 1;
	}
	return true;
}

inline string JoinString(vector<string>& strArray, string delim) {
	string s = "";
	for (size_t i = 0; i < strArray.size(); ++i) {
		s += strArray[i];
		if (i != strArray.size() - 1) {
			s += delim;
		}
	}
	return s;
}

inline string& ReplaceAll(string& src, const string& oldValue, const string& newValue) {
	for (string::size_type pos(0); pos != string::npos; pos += newValue.length()) {
		if ((pos = src.find(oldValue, pos)) != string::npos) {
			src.replace(pos, oldValue.length(), newValue);
		} else {
			break;
		}
	}
	return src;
}

inline size_t strlcpy(char* d, size_t n, char const* s) {
	return snprintf(d, n, "%s", s);
}

// binance
inline string encryptWithHMAC(string key, string data) {
    unsigned char *result;
    static char res_hexstring[64];
    int result_len = 32;
    string signature;

    result = HMAC(EVP_sha256(), key.c_str(),key.length(), const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(data.c_str())), data.length(), NULL, NULL);
  	for (int i = 0; i < result_len; i++) {
    	sprintf(&(res_hexstring[i * 2]), "%02x", result[i]);
  	}

  	for (int i = 0; i < 64; i++) {
  		signature += res_hexstring[i];
  	}

  	return signature;
}

inline string getSignature(string query, string apiSecret) {
	return encryptWithHMAC(apiSecret.c_str(), query.c_str());
}

// coinbase
static std::string const base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

inline std::string base64_encode(unsigned char const * input, size_t len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (len--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++) {
                ret += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) {
            ret += base64_chars[char_array_4[j]];
        }

        while((i++ < 3)) {
            ret += '=';
        }
    }

    return ret;
}

inline string base64_encode(string input) {
    return base64_encode(
            reinterpret_cast<const unsigned char *>(input.data()),
            input.size()
    );
}

inline int HmacEncode(const char * algo,
               const char * key, unsigned int key_length,
               const char * input, unsigned int input_length,
               unsigned char * &output, unsigned int &output_length) {
    const EVP_MD * engine = NULL;
    if(strcasecmp("sha512", algo) == 0) {
        engine = EVP_sha512();
    }
    else if(strcasecmp("sha256", algo) == 0) {
        engine = EVP_sha256();
    }
    else if(strcasecmp("sha1", algo) == 0) {
        engine = EVP_sha1();
    }
    else if(strcasecmp("md5", algo) == 0) {
        engine = EVP_md5();
    }
    else if(strcasecmp("sha224", algo) == 0) {
        engine = EVP_sha224();
    }
    else if(strcasecmp("sha384", algo) == 0) {
        engine = EVP_sha384();
    }
    else if(strcasecmp("sha", algo) == 0) {
        //engine = EVP_sha();
        assert(0);
    }
    else {
        cout << "Algorithm " << algo << " is not supported by this program!" << endl;
        return -1;
    }

    output = (unsigned char*)malloc(EVP_MAX_MD_SIZE);

    //HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, strlen(key), engine, NULL);
    HMAC_Update(&ctx, (unsigned char*)input, strlen(input));        // input is OK; &input is WRONG !!!

    HMAC_Final(&ctx, output, &output_length);
    //HMAC_CTX_free(ctx);

    return 0;
}

inline string getSignatureNew(string query, string apiSecret) {  // coinbase
    unsigned char * mac = NULL;
    unsigned int mac_length = 0;
    int ret = HmacEncode("sha256", apiSecret.c_str(), apiSecret.length(), query.c_str(), query.length(), mac, mac_length);
    string signature = base64_encode(mac, mac_length);
    return signature;
}


// gateio
inline string encryptWithHMACGate(string key, string data) {
    unsigned char* digest = HMAC(EVP_sha512(), key.c_str(), key.length(), const_cast<unsigned char *>(reinterpret_cast<const unsigned char*>(data.c_str())), data.length(), NULL, NULL);
    char mdString[256];
    for(int i = 0; i < 64; ++i)
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

    string signature = mdString;
    return signature;
}


inline string getSignatureGate(string query, string apiSecret) {
	return encryptWithHMACGate(apiSecret.c_str(), query.c_str());
}

inline string sha512(string data) {
    unsigned char result[SHA512_DIGEST_LENGTH] = {0};
    EVP_Digest(data.c_str(), data.length(), result, NULL, EVP_sha512(), NULL);

    char c[20];
    stringstream ss;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++){
        sprintf(c, "%02x", result[i]);
        ss << c;
    }

    return ss.str();
}

inline string GetDeliveryContractSymbol(string symbol, string contractType, tm curDate) {
	string s = symbol;
	vector<string> v;
	SplitString(s, "-", v);
	if (v.size() >= 3) {
		return s;
	} else if (v.size() >= 2) {
		string quote = v[1];
		if (quote == "USD") {
			char sym[64];
			if (contractType == "cq") {
				if (curDate.tm_mon + 1 <= 3) {
					sprintf(sym, "%s-%02d%02d%02d", s.c_str(), (curDate.tm_year + 1900) % 100, 3, 31);
				} else if (curDate.tm_mon + 1 >= 4 && curDate.tm_mon + 1 <= 6) {
					sprintf(sym, "%s-%02d%02d%02d", s.c_str(), (curDate.tm_year + 1900) % 100, 6, 30);
				}  else if (curDate.tm_mon + 1 >= 7 && curDate.tm_mon + 1 <= 9) {
					sprintf(sym, "%s-%02d%02d%02d", s.c_str(), (curDate.tm_year + 1900) % 100, 9, 30);
				} else if (curDate.tm_mon + 1 >= 10 && curDate.tm_mon + 1 <= 12) {
					sprintf(sym, "%s-%02d%02d%02d", s.c_str(), (curDate.tm_year + 1900) % 100, 12, 30);
				}
			} else if (contractType == "nq") {
				if (curDate.tm_mon + 1 <= 3) {
					sprintf(sym, "%s-%02d%02d%02d", s.c_str(), (curDate.tm_year + 1900) % 100, 6, 30);
				} else if (curDate.tm_mon + 1 >= 4 && curDate.tm_mon + 1 <= 6) {
					sprintf(sym, "%s-%02d%02d%02d", s.c_str(), (curDate.tm_year + 1900) % 100, 9, 30);
				}  else if (curDate.tm_mon + 1 >= 7 && curDate.tm_mon + 1 <= 9) {
					sprintf(sym, "%s-%02d%02d%02d", s.c_str(), (curDate.tm_year + 1900) % 100, 12, 30);
				} else if (curDate.tm_mon + 1 >= 10 && curDate.tm_mon + 1 <= 12) {
					sprintf(sym, "%s-%02d%02d%02d", s.c_str(), (curDate.tm_year + 1900 + 1) % 100, 3, 31);
				}
			}
			return sym;
		}
	}

	return s;
}

#define BINANCE_SPOT_REST "https://api.binance.com"
#define BINANCE_UFUTURE_REST "https://fapi.binance.com"
#define BINANCE_CFUTURE_REST "https://dapi.binance.com"

#define GATEIO_REST "https://api.gateio.ws/api/v4"

#define BYBIT_REST "https://api.bybit.com"

#define OKX_REST "https://www.okx.com"