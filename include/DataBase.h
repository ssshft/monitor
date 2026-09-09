#pragma once

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include "Utility.h"
#include <mysql/mysql.h>


class DataBase {
public:
	DataBase();
	virtual ~DataBase();
	void FreeConnect();
    bool ConnectDatabase();
	bool ConnectDatabase(std::string dbName);
	bool ConnectDatabase(std::string host, std::string user, std::string password, std::string dbName, int port);
	void CreateRiskInfoTable();
	void RunSql(std::string sql);
	void InsertRiskInfo(vector<igmonitor::RiskInfo>& vRiskInfo);

private:
	MYSQL *pSql;
	bool connected;
	std::string dbHost;
    std::string dbUser; 
    std::string dbPassword;
	int dbPort;
	std::string dbName;
	std::string dbRiskInfoTable;
};
