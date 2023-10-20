#include <string>
#include <map>
#include <iostream>
#include <thread>
#include <unistd.h>

using namespace  std;

string GetCmdResult(const string& cmd) {
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

int  main( int  argc,  char * argv[])
{
     string cmd = "ping -c 5 172.31.43.30";
     string res = GetCmdResult(cmd);
     if (res.find("0 received") != string::npos) {
          std::cout << "server connot pind!" << std::endl;
     } else {
          std::cout << "connected!" << std::endl;
     }
     return 0;
}