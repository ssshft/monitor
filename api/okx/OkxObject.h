#pragma once

#include <string>
#include <sstream>

using namespace std;


namespace okx {
    struct OkxAsset {
        string ccy{""};
        double eq{0.0};
        double availableBal{0.0};
        double availableEq{0.0};
        double cashBal{0.0};
        double frozenBal{0.0};
        double liab{0.0};
        double upl{0.0};

	    string toString() {
	        stringstream ss;
	        ss << "ccy:" << ccy << " eq:" << eq << " availableBal:" << availableBal << " availableEq:" << availableEq << " cashBal:" << cashBal << " frozenBal:" << frozenBal << " liab:" << liab << " upl:" << upl;
	        return ss.str();
	    }
    };

    struct OkxPosition {
        string instId{""};
        string instType{""};
        double pos{0.0};
        double mmr{0.0};
        double avgPx{0.0};
        double upl{0.0};
        double markPx{0.0};
        double liqPx{0.0};
        double adl{0.0};

        string toString() {
            stringstream ss;
            ss << "instId:" << instId << " instType:" << instType << " pos:" << pos << " mmr:" << mmr << " avgPx:" << avgPx << " upl:" << upl << " markPx:" << markPx << " liqPx:" << liqPx << " adl:" << adl;
            return ss.str();
        }
    };

    struct OkxOrder {
        string instId{""};
        string instType{""};
        double sz;
        double accFillSz;
        double px;
        double avgPx;
        string state;
        string side;
        string category;
        int64_t updateTime;
        int64_t createTime;

        string toString() {
            stringstream ss;
            ss << "instId:" << instId << " instType:" << instType << " sz:" << sz << " accFillSz:" << accFillSz << " px:" << px << " avgPx:" << avgPx << " state:" << state << " side:" << side << " category:" << category << " updateTime:" << updateTime << " createTime:" << createTime;
            return ss.str();
        }
    };
}
