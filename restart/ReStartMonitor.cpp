#include "program_util.h"
#include "Utility.h"
#include <iostream>
#include <string>

using namespace std;


int main() {
    string program = "risk_monitor";
    while (1) {
        if(crypto::ensure_one_instance(program)) {
            string currentTime = CovertToUtcStr(GetCurrentTimeUs());
            std::cout << currentTime << " " << program << " not exist! need restart!" << std::endl;
            string command = "./start_risk_monitor.sh";
            int res = system(command.c_str());
            std::cout << "start command: " << command << " result: " << res << std::endl;
        }
        sleep(5);
    }
    return 0;
}