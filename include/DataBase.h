#pragma once

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include "Utility.h"
#include <mysql/mysql.h>

using namespace std;

class DataBase {
public:
	DataBase();
	virtual ~DataBase();
	void FreeConnect();
    bool ConnectDatabase();
	bool ConnectDatabase(string dbName);
	bool ConnectDatabase(string host, string user, string password, string dbName, int port);
	void CreateRiskInfoTable();
	void RunSql(string sql);
	vector<DbOrder> QueryOrder(int status, string msg="");
	vector<DbOrder> QueryOrder(string startTime, string endTime);
	void InsertRiskInfo(vector<igmonitor::RiskInfo>& vRiskInfo);
	unordered_map<string, SymbolMarketInfo> QueryMarketInfoVolume(string host, string user, string password, string dbName, int port);
	unordered_map<string, SymbolMarketInfo> QueryMarketInfoOpenInterest(string host, string user, string password, string dbName, int port);

private:
	MYSQL *pSql;
	bool connected;
	string dbHost;
    string dbUser; 
    string dbPassword;
	int dbPort;
	string dbName;
	string dbOrdTable;
	string dbOrdDetailTable;
	string dbRiskInfoTable;
};
