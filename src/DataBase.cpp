#include "DataBase.h"
#include "MonitorConfig.h"
#include "BasicInfoMgr.h"


DataBase::DataBase() {
	pSql = nullptr;
	connected = false;
	dbHost = MonitorConfig::GetInstance().GetMysqlAddr();
	dbUser = MonitorConfig::GetInstance().GetMysqlUser();
	dbPassword = MonitorConfig::GetInstance().GetMysqlPassword();
	dbPort = MonitorConfig::GetInstance().GetMysqlPort();
	dbName = MonitorConfig::GetInstance().GetMysqlDbName();
	dbOrdTable = MonitorConfig::GetInstance().GetMysqlOrdTableName();
	dbOrdDetailTable = MonitorConfig::GetInstance().GetMysqlOrdDetailTableName();
	dbRiskInfoTable = MonitorConfig::GetInstance().GetMysqlRiskInfoTableName();
}

DataBase::~DataBase() {
	FreeConnect();
}

void DataBase::FreeConnect() {
	if(pSql) {
		mysql_close(pSql);
		pSql = nullptr;
		connected = false;
	}
}

bool DataBase::ConnectDatabase() {
	FreeConnect();
	pSql = mysql_init((MYSQL*)0);
	if (pSql) {
		if (0 != mysql_options(pSql, MYSQL_SET_CHARSET_NAME, "GB2312")) {
			LOG_INFO("mysql_options GB2312 error!");
		}

		if (mysql_real_connect(pSql, dbHost.c_str(), dbUser.c_str(), dbPassword.c_str(), dbName.c_str(), dbPort, nullptr, 0) != nullptr) {
			connected = true;
		}
		else {
			connected = false;
			stringstream ss;
			ss << "mysql_real_connect error! host: " << dbHost << " user: " << dbUser << " password: " << dbPassword << " dbName: " << dbName << " port: " << dbPort;
			LOG_INFO("Fail when run sql: %s", ss.str().c_str());
		}
	}
	else {
		LOG_INFO("mysql_init error");
	}

	return connected;
}

bool DataBase::ConnectDatabase(string dbName) {
	FreeConnect();
	pSql = mysql_init((MYSQL*)0);
	if (pSql) {
		if (0 != mysql_options(pSql, MYSQL_SET_CHARSET_NAME, "GB2312")) {
			LOG_INFO("mysql_options GB2312 error!");
		}

		if (mysql_real_connect(pSql, dbHost.c_str(), dbUser.c_str(), dbPassword.c_str(), dbName.c_str(), dbPort, nullptr, 0) != nullptr) {
			connected = true;
		}
		else {
			connected = false;
			stringstream ss;
			ss << "mysql_real_connect error! host: " << dbHost << " user: " << dbUser << " password: " << dbPassword << " dbName: " << dbName << " port: " << dbPort;
			LOG_INFO("Fail when run sql: %s", ss.str().c_str());
		}
	}
	else {
		LOG_INFO("mysql_init error");
	}

	return connected;
}

bool DataBase::ConnectDatabase(string host, string user, string password, string dbName, int port) {
	FreeConnect();
	pSql = mysql_init((MYSQL*)0);
	if (pSql) {
		if (0 != mysql_options(pSql, MYSQL_SET_CHARSET_NAME, "GB2312")) {
			LOG_INFO("mysql_options GB2312 error!");
		}

		if (mysql_real_connect(pSql, host.c_str(), user.c_str(), password.c_str(), dbName.c_str(), port, nullptr, 0) != nullptr) {
			connected = true;
		} else {
			connected = false;
			stringstream ss;
			ss << "mysql_real_connect error! host: " << host << " user: " << user << " password: " << password << " dbName: " << dbName << " port: " << port;
			LOG_INFO("Fail when run sql: %s", ss.str().c_str());
		}
	} else {
		LOG_INFO("mysql_init error");
	}

	return connected;
}

void DataBase::CreateRiskInfoTable() {
	bool ret = ConnectDatabase(dbHost, dbUser, dbPassword, dbName, dbPort);
	if (ret) {
        stringstream sql;
        sql << "create table " << dbRiskInfoTable << "(pk_id bigint unsigned primary key auto_increment, account_id int, account_name varchar(128), base_asset varchar(32), net_value decimal(30,15), risk_exposure decimal(30,15), leverage decimal(30,15), gmt_create datetime default current_timestamp, gmt_modified datetime default current_timestamp on update current_timestamp)";
        RunSql(sql.str().c_str());
    }
	FreeConnect();
}

void DataBase::RunSql(string sql) {
	if (pSql) {
		if (mysql_query(pSql, sql.c_str()) != 0) {
			LOG_INFO("Fail when run sql: %s", sql.c_str());
		}
	} else {
		LOG_INFO("Have not connected database!");
	}
}

vector<DbOrder> DataBase::QueryOrder(int status, string msg) {
	vector<DbOrder> v;

	if (dbOrdDetailTable.size() <= 0 || dbOrdTable.size() <= 0) {
		return v;
	}

	bool ret = ConnectDatabase(dbHost, dbUser, dbPassword, dbName, dbPort);
	if (ret) {
		int64_t currentUs = GetCurrentTimeUs();
		int64_t oneMinute = 60 * 1000 * 1000;
		int64_t twoMinute = 2 * oneMinute;
		int64_t beforeCurrentOneMinute = currentUs - oneMinute;
		int64_t beforeCurrentTwoMinute = currentUs - twoMinute;

		string beforeOneMinuteStr = CovertToUtcStr(beforeCurrentOneMinute, false);
		string beforeTwoMinuteStr = CovertToUtcStr(beforeCurrentTwoMinute, false);

		stringstream sql;
		if (msg.length() > 0) {
			sql << "select * from " << dbOrdDetailTable << " where status=" << status << " and err_msg like '%" << msg << "%' and client_order_id in (select client_order_id from " << dbOrdTable << " where status=" << status << " and event_time>='" << beforeTwoMinuteStr << "' and event_time<='" << beforeOneMinuteStr << "' and client_order_id!='') and event_time>='" << beforeTwoMinuteStr << "' and event_time<='" << beforeOneMinuteStr << "'";
		} else {
			sql << "select * from " << dbOrdDetailTable << " where status=" << status << " and client_order_id in (select client_order_id from " << dbOrdTable << " where status=" << status << " and event_time>='" << beforeTwoMinuteStr << "' and event_time<='" << beforeOneMinuteStr << "' and client_order_id!='') and event_time>='" << beforeTwoMinuteStr << "' and event_time<='" << beforeOneMinuteStr << "'";
		}

		if (mysql_query(pSql, sql.str().c_str()) != 0) {
			LOG_INFO("Fail when run sql: %s", sql.str().c_str());
			return v;
		}

		MYSQL_RES* pRes = mysql_store_result(pSql);
		if (pRes == nullptr) {
			LOG_INFO("mysql_store_result error");
			return v;
		}

		int colNum = mysql_num_fields(pRes);  
		MYSQL_ROW pRow;
		while (pRow = mysql_fetch_row(pRes)) {  
			DbOrder order;
			order.cmdId = stoi(pRow[1]);
			order.accountId = stoi(pRow[2]);
			order.customerId = stoi(pRow[3]);
			order.accountType = stoi(pRow[4]);
			order.orderId = stoll(pRow[5]);
			order.clientOrderId = pRow[6];
			order.symbol = pRow[7];
			order.exchangeId = stoi(pRow[8]);
			order.side = pRow[9];
			order.price = stod(pRow[10]);
			order.quantity = stod(pRow[11]);
			order.cumuFilledQuantity = stod(pRow[12]);
			order.status = stoi(pRow[13]);
			order.statusMsg = pRow[14];
			order.avgPrice = stod(pRow[15]);
			order.commissionAmount = stod(pRow[16]);
			order.commissionAsset = pRow[17];
			order.lastExecutedQuantity = stod(pRow[18]);
			order.lastExecutedPrice = stod(pRow[19]);
			order.eventTime = pRow[20];
			order.errType = stoi(pRow[21]);
			order.errCode = stoi(pRow[22]);
			order.errMsg = pRow[23];
			v.emplace_back(order);
		}
		mysql_free_result(pRes);
	}
	FreeConnect();
	return v;
}

vector<DbOrder> DataBase::QueryOrder(string startTime, string endTime) {
	vector<DbOrder> v;

	if (dbOrdTable.size() <= 0) {
		return v;
	}

	bool ret = ConnectDatabase(dbHost, dbUser, dbPassword, dbName, dbPort);
	if (ret) {
		stringstream sql;
		sql << "(select * from " << dbOrdTable << " where event_time>='" << startTime << "' and event_time<='" << endTime << "')";

		if (mysql_query(pSql, sql.str().c_str()) != 0) {
			LOG_INFO("Fail when run sql: %s", sql.str().c_str());
			return v;
		}

		MYSQL_RES* pRes = mysql_store_result(pSql);
		if (pRes == nullptr) {
			LOG_INFO("mysql_store_result error");
			return v;
		}

		int colNum = mysql_num_fields(pRes);  
		MYSQL_ROW pRow;
		while (pRow = mysql_fetch_row(pRes)) {  
			DbOrder order;
			order.cmdId = stoi(pRow[1]);
			order.accountId = stoi(pRow[2]);
			order.customerId = stoi(pRow[3]);
			order.accountType = stoi(pRow[4]);
			order.orderId = stoll(pRow[5]);
			order.clientOrderId = pRow[6];
			order.symbol = pRow[7];
			order.exchangeId = stoi(pRow[8]);
			order.side = pRow[9];
			order.price = stod(pRow[10]);
			order.quantity = stod(pRow[11]);
			order.cumuFilledQuantity = stod(pRow[12]);
			order.status = stoi(pRow[13]);
			order.statusMsg = pRow[14];
			order.avgPrice = stod(pRow[15]);
			order.commissionAmount = stod(pRow[16]);
			order.commissionAsset = pRow[17];
			order.eventTime = pRow[18];
			v.emplace_back(order);
		}
		mysql_free_result(pRes);
	}
	FreeConnect();

	return v;
}

void DataBase::InsertRiskInfo(vector<igmonitor::RiskInfo>& vRiskInfo) {
	if (dbRiskInfoTable.size() <= 0) {
		return;
	}

	if (vRiskInfo.size() <= 0) {
		return;
	}

	bool ret = ConnectDatabase(dbHost, dbUser, dbPassword, dbName, dbPort);
	if (ret) {
		for (size_t i = 0; i < vRiskInfo.size(); ++i) {
			igmonitor::RiskInfo riskInfo = vRiskInfo[i];
			stringstream sqlInsert;
		    sqlInsert << "insert into " << dbRiskInfoTable << "(account_id,account_name,base_asset,net_value,risk_exposure,leverage) values(" << riskInfo.accountId << ",'" << riskInfo.name << "','" << riskInfo.baseAsset << "'," << riskInfo.netValueD << "," << riskInfo.riskExposureD << "," << riskInfo.maxRealLeverageD << ")"; 
		    if (mysql_query(pSql, sqlInsert.str().c_str()) != 0) {
			    LOG_INFO("Fail when run sql: %s", sqlInsert.str().c_str());
		    }
		}
	}

	FreeConnect();
}

unordered_map<string, SymbolMarketInfo> DataBase::QueryMarketInfoVolume(string host, string user, string password, string dbName, int port) {
	unordered_map<string, SymbolMarketInfo> m;
	bool ret = ConnectDatabase(host, user, password, dbName, port);
	if (ret) {
		int64_t currentUs = GetCurrentTimeUs();
		int64_t oneMinute = 60 * 1000 * 1000;
		int64_t beforeDayUs = currentUs - 24 * 60 * oneMinute;
		string beforeDay = CovertToUtcDateStr(beforeDayUs);

		stringstream sqlSpot;
		sqlSpot << "select * from `binance|allcoin-usdt|spot` where openTime_read='" << beforeDay << "'";
		if (mysql_query(pSql, sqlSpot.str().c_str()) == 0) {
			MYSQL_RES* pRes = mysql_store_result(pSql);
			if (pRes != nullptr) {
				MYSQL_ROW pRow;
				while (pRow = mysql_fetch_row(pRes)) {  
					SymbolMarketInfo symbolMarketInfo;
					symbolMarketInfo.symbol = pRow[0];
					symbolMarketInfo.volume = stod(pRow[7]);
					string key = "BINANCE|" + symbolMarketInfo.symbol + "|SPOT";
					m[key] = symbolMarketInfo;
				}
				mysql_free_result(pRes);
			} else {
				LOG_INFO("mysql_store_result error, when run sql: %s", sqlSpot.str().c_str());
			}
		} else {
			LOG_INFO("Fail when run sql: %s", sqlSpot.str().c_str());
		}


		stringstream sqlCm;
		sqlCm << "select * from `binance|allcoin|cm` where openTime_read='" << beforeDay << "'";
		if (mysql_query(pSql, sqlCm.str().c_str()) == 0) {
			MYSQL_RES* pRes = mysql_store_result(pSql);
			if (pRes != nullptr) {
				MYSQL_ROW pRow;
				while (pRow = mysql_fetch_row(pRes)) {  
					SymbolMarketInfo symbolMarketInfo;
					symbolMarketInfo.symbol = pRow[0];
					double open = stod(pRow[3]);
					//double high = stod(pRow[4]);
					//double low = stod(pRow[5]);
					double close = stod(pRow[6]);
					vector<string> v;
					SplitString(symbolMarketInfo.symbol, "-", v);
					string key = "";
					if (v.size() == 3) {
						key = "BINANCE|" + symbolMarketInfo.symbol + "|FUTURES";
					} else {
						key = "BINANCE|" + symbolMarketInfo.symbol + "|SWAP";
					}
					InstrumentInfo& info = BasicInfoMgr::GetInstance().GetBasicInfo(key);
					if (info.multiple > 0) {
						symbolMarketInfo.volume = stod(pRow[7]) * (open + close) / 2 / info.multiple;
						m[key] = symbolMarketInfo;
					}
					
				}
				mysql_free_result(pRes);
			} else {
				LOG_INFO("mysql_store_result error, when run sql: %s", sqlCm.str().c_str());
			}
		} else {
			LOG_INFO("Fail when run sql: %s", sqlCm.str().c_str());
		}


		stringstream sqlUm;
		sqlUm << "select * from `binance|allcoin|um` where openTime_read='" << beforeDay << "'";
		if (mysql_query(pSql, sqlUm.str().c_str()) == 0) {
			MYSQL_RES* pRes = mysql_store_result(pSql);
			if (pRes != nullptr) {
				MYSQL_ROW pRow;
				while (pRow = mysql_fetch_row(pRes)) {  
					SymbolMarketInfo symbolMarketInfo;
					symbolMarketInfo.symbol = pRow[0];
					symbolMarketInfo.volume = stod(pRow[7]);
					vector<string> v;
					SplitString(symbolMarketInfo.symbol, "-", v);
					string key = "";
					if (v.size() == 3) {
						key = "BINANCE|" + symbolMarketInfo.symbol + "|FUTURES";
					} else {
						key = "BINANCE|" + symbolMarketInfo.symbol + "|SWAP";
					}
					m[key] = symbolMarketInfo;
				}
				mysql_free_result(pRes);
			} else {
				LOG_INFO("mysql_store_result error, when run sql: %s", sqlUm.str().c_str());
			}
		} else {
			LOG_INFO("Fail when run sql: %s", sqlUm.str().c_str());
		}
	}

	FreeConnect();
	return m;
}

unordered_map<string, SymbolMarketInfo> DataBase::QueryMarketInfoOpenInterest(string host, string user, string password, string dbName, int port) {
	unordered_map<string, SymbolMarketInfo> m;
	bool ret = ConnectDatabase(host, user, password, dbName, port);
	if (ret) {
		int64_t currentUs = GetCurrentTimeUs();
		int64_t oneMinute = 60 * 1000 * 1000;
		int64_t beforeDayUs = currentUs - 24 * 60 * oneMinute;
		tm beforeDay = CovertToUtcTm(beforeDayUs);
		char table[64];
		sprintf(table, "binance|%04d-%02d-%02d|umcm", beforeDay.tm_year + 1900, beforeDay.tm_mon + 1, beforeDay.tm_mday);
		char timeRead[32];
		sprintf(timeRead, "%d-%d-%d 23:55", beforeDay.tm_year + 1900, beforeDay.tm_mon + 1, beforeDay.tm_mday);

		stringstream sql;
		sql << "select * from `" << table << "` where time_read='" << timeRead << "'";
		if (mysql_query(pSql, sql.str().c_str()) == 0) {
			MYSQL_RES* pRes = mysql_store_result(pSql);
			if (pRes != nullptr) {
				MYSQL_ROW pRow;
				while (pRow = mysql_fetch_row(pRes)) {
					string symbol = pRow[0];
					double perpOpenInterest = 0;
					if (pRow[14]) {
						perpOpenInterest = stod(pRow[14]);
					}

					double cqOpenInterest = 0;
					if (pRow[16]) {
						cqOpenInterest = stod(pRow[16]);
					}
					
					double nqOpenInterest = 0;
					if (pRow[18]) {
						nqOpenInterest = stod(pRow[18]);
					}
					
					vector<string> v;
					SplitString(symbol, "-", v);
					string key = "";
					if (v.size() == 3) {
						key = "BINANCE|" + symbol + "|FUTURES";
					} else {
						key = "BINANCE|" + symbol + "|SWAP";
					}

					if (perpOpenInterest > 0) {
						SymbolMarketInfo symbolMarketInfo;
						symbolMarketInfo.symbol = symbol;
						symbolMarketInfo.openInterest = perpOpenInterest;
						m[key] = symbolMarketInfo;
					}

					if (cqOpenInterest > 0) {
						string curQ = GetDeliveryContractSymbol(symbol, "cq", beforeDay);
						SymbolMarketInfo symbolMarketInfo;
						symbolMarketInfo.symbol = curQ;
						symbolMarketInfo.openInterest = cqOpenInterest;
						string k = "BINANCE|" + curQ + "|FUTURES";
						m[k] = symbolMarketInfo;
					}

					if (nqOpenInterest > 0) {
						string nextQ = GetDeliveryContractSymbol(symbol, "nq", beforeDay);
						SymbolMarketInfo symbolMarketInfo;
						symbolMarketInfo.symbol = nextQ;
						symbolMarketInfo.openInterest = nqOpenInterest;
						string k = "BINANCE|" + nextQ + "|FUTURES";
						m[k] = symbolMarketInfo;
					}
				}
				mysql_free_result(pRes);
			} else {
				LOG_INFO("mysql_store_result error, when run sql: %s", sql.str().c_str());
			}
		} else {
			LOG_INFO("Fail when run sql: %s", sql.str().c_str());
		}
	}

	FreeConnect();
	return m;
}