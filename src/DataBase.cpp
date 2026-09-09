#include "DataBase.h"
#include "MonitorConfig.h"


DataBase::DataBase() {
	pSql = nullptr;
	connected = false;
	dbHost = MonitorConfig::GetInstance().GetMysqlAddr();
	dbUser = MonitorConfig::GetInstance().GetMysqlUser();
	dbPassword = MonitorConfig::GetInstance().GetMysqlPassword();
	dbPort = MonitorConfig::GetInstance().GetMysqlPort();
	dbName = MonitorConfig::GetInstance().GetMysqlDbName();
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