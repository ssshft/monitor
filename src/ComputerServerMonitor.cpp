#include "ComputerServerMonitor.h"
#include <unistd.h>
#include <stdlib.h>


ComputerServerMonitor::ComputerServerMonitor() {

}

ComputerServerMonitor::~ComputerServerMonitor() {

}

string ComputerServerMonitor::GetCmdResult(const string& cmd) {
    string res = "";
    char buf[1024] = {0};
    FILE* pf;

    if ((pf = popen(cmd.c_str(), "r")) == NULL) {
        return res;
    }

    while (fgets(buf, sizeof(buf), pf)) {
        res += buf;
    }

    pclose(pf);

    unsigned int size = res.size();
    if (size > 0 && res[size - 1] == '\n') {
        res = res.substr(0, size - 1);
    }

    return res;
}

bool ComputerServerMonitor::Ping(const string& ip) {
    bool f = false;
    string cmd = "ping -c 5 " + ip;
    string res = GetCmdResult(cmd);

    if (res.find("0 received") != string::npos) {
        f = false;
     } else {
        f = true;
     }
     return f;
}