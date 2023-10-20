#pragma once

#include <string>

using namespace std;


class ComputerServerMonitor {
public:
    ComputerServerMonitor();
    ~ComputerServerMonitor();
    string GetCmdResult(const string& cmd);
    bool Ping(const string& ip);
};
