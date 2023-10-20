#ifndef _SQLITE_H
#define _SQLITE_H

#include "sqlite3.h"

namespace db {
    class Sqlite {
    public:
        Sqlite() {
            dbName = "";
            db = nullptr;
		    connected = false;
        };

        Sqlite(const char* name) {
            dbName = name;
            db = nullptr;
		    connected = false;
        };

        ~Sqlite() {
            FreeConnect();
        };

        void FreeConnect() {
	        if(db) {
                sqlite3_close(db);
		        db = nullptr;
		        connected = false;
	        }
        }

        bool ConnectDatabase() {
	        FreeConnect();
            char *zErrMsg = 0;
            int rc = sqlite3_open(dbName, &db);
            if (rc) {
                LOG_INFO("Can't open database: %s", sqlite3_errmsg(db));
                connected = false;
            } else {
                LOG_INFO("Opened database successfully");
                connected = true;
            }
	        return connected;
        }

        bool ConnectDatabase(const char* name) {
	        FreeConnect();
            char *zErrMsg = 0;
            int rc = sqlite3_open(name, &db);
            if (rc) {
                LOG_INFO("Can't open database: %s", sqlite3_errmsg(db));
                connected = false;
            } else {
                LOG_INFO("Opened database successfully");
                connected = true;
            }
	        return connected;
        }

        bool RunSql(const char* sql) {
            bool res = false;
            if (connected) {
                char *zErrMsg = 0;
                int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
                if( rc != SQLITE_OK ){
                    LOG_INFO("SQL error: %s", zErrMsg);
                    sqlite3_free(zErrMsg);
                    res = false;
                } else {
                    LOG_INFO("Run Sql %s successfully", sql);
                    res = true;
                }
            }
            return res;
        }

        vector<vector<string>> QueryAllColumnsArray(string tableName) {
            vector<vector<string>> v;
            bool con = ConnectDatabase();
            if (con) {
                char** result = 0;
                char* errmsg = 0;
                int nRow = 0;
                int nColumn = 0;
                string sql = "select * from " + tableName;
                if (sqlite3_get_table(db, sql.c_str(), &result, &nRow, &nColumn, &errmsg) != 0) {
                    LOG_INFO("SQL error: %s", errmsg);
                    sqlite3_free(errmsg);
                } else {
                    int index = nColumn;
                    for (int i = 0; i < nRow; ++i) {
                        vector<string> vRow;
                        for (int j = 0; j < nColumn; ++j) {
                            string colName = result[j];
                            string value = result[index];
                            vRow.emplace_back(value);
                            index++;
                        }
                        v.emplace_back(vRow);
                    }
                    sqlite3_free_table(result);
                }
            }
            FreeConnect();
            return v;
        }

        vector<unordered_map<string, string>> QueryAllColumnsMap(string tableName) {
            vector<unordered_map<string, string>> v;
            bool con = ConnectDatabase();
            if (con) {
                char** result = 0;
                char* errmsg = 0;
                int nRow = 0;
                int nColumn = 0;
                string sql = "select * from " + tableName;
                if (sqlite3_get_table(db, sql.c_str(), &result, &nRow, &nColumn, &errmsg) != 0) {
                    LOG_INFO("SQL error: %s", errmsg);
                    sqlite3_free(errmsg);
                } else {
                    int index = nColumn;
                    for (int i = 0; i < nRow; ++i) {
                        unordered_map<string, string> mRow;
                        for (int j = 0; j < nColumn; ++j) {
                            string colName = result[j];
                            string value = result[index];
                            mRow[colName] = value;
                            index++;
                        }
                        v.emplace_back(mRow);
                    }
                    sqlite3_free_table(result);
                }
            }
            FreeConnect();
            return v;
        }

        vector<vector<string>> QueryColumnsArray(string sql) {
            vector<vector<string>> v;
            bool con = ConnectDatabase();
            if (con) {
                char** result = 0;
                char* errmsg = 0;
                int nRow = 0;
                int nColumn = 0;
                if (sqlite3_get_table(db, sql.c_str(), &result, &nRow, &nColumn, &errmsg) != 0) {
                    LOG_INFO("SQL error: %s", errmsg);
                    sqlite3_free(errmsg);
                } else {
                    int index = nColumn;
                    for (int i = 0; i < nRow; ++i) {
                        vector<string> vRow;
                        for (int j = 0; j < nColumn; ++j) {
                            string colName = result[j];
                            string value = result[index];
                            vRow.emplace_back(value);
                            index++;
                        }
                        v.emplace_back(vRow);
                    }
                    sqlite3_free_table(result);
                }
            }
            FreeConnect();
            return v;
        }

        vector<unordered_map<string, string>> QueryColumnsMap(string sql) {
            vector<unordered_map<string, string>> v;
            bool con = ConnectDatabase();
            if (con) {
                char** result = 0;
                char* errmsg = 0;
                int nRow = 0;
                int nColumn = 0;
                if (sqlite3_get_table(db, sql.c_str(), &result, &nRow, &nColumn, &errmsg) != 0) {
                    LOG_INFO("SQL error: %s", errmsg);
                    sqlite3_free(errmsg);
                } else {
                    int index = nColumn;
                    for (int i = 0; i < nRow; ++i) {
                        unordered_map<string, string> mRow;
                        for (int j = 0; j < nColumn; ++j) {
                            string colName = result[j];
                            string value = result[index];
                            mRow[colName] = value;
                            index++;
                        }
                        v.emplace_back(mRow);
                    }
                    sqlite3_free_table(result);
                }
            }
            FreeConnect();
            return v;
        }

    private:
        const char* dbName;
        sqlite3 *db;
        bool connected;
    };
}


#endif