#pragma once

#include <string>

using namespace std;

namespace coinbase {
    struct SpotAsset {
        string symbol{""};
        double amount{0.0};
        double holds{0.0};
        double bondedAmount{0.0};
        double reservedAmount{0.0};
        double unbondingAmount{0.0};
        double unvestedAmount{0.0};
        double pendingRewardsAmount{0.0};
        double pastRewardsAmount{0.0};
        double bondableAmount{0.0};
        double withdrawableAmount{0.0};

	string toString() {
	    stringstream ss;
	    ss << "symbol:" << symbol << " amount:" << amount << " holds:" << holds << " bondedAmount:" << bondedAmount << " reservedAmount:" << reservedAmount << " unbondingAmount:" << unbondingAmount << " unvestedAmount:" << unvestedAmount << " pastRewardsAmount:" << pastRewardsAmount << " bondableAmount:" << bondableAmount <<  " withdrawableAmount:" << withdrawableAmount;
	    return ss.str();
	}
    };
}
