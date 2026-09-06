#include "MonitorConfig.h"
#include "program_util.h"
#include "crypto_exception.h"
#include "MonitorOperation.h"

std::mutex mut;


int main(int argc, char* argv[]) {
    std::string currentTimeStr = CovertToUtcStr(crypto::getCurrentTime(), false);
    std::cout << currentTimeStr << " monitor server executed..." << std::endl;

    for (auto i = 0; i < argc; ++i) {
        std::cout << currentTimeStr << " argv[" << i << "]:" << argv[i] << std::endl;
    }

    if (argc < 2) {
        std::cout << "Incorrect parameters, please check!" << std::endl;
        return 0;
    }

    ::chdir(argv[1]);

    MonitorConfig::GetInstance().LoadConfig();
    std::string tag = MonitorConfig::GetInstance().GetLogTag();;
    std::string program = tag;
    long currentPid = getpid();
    long filePid = crypto::get_program_pid(program);
    if(!crypto::ensure_one_instance(program) && currentPid != filePid) {
        std::string errormsg = tag + " with pid=" + std::to_string(filePid) + " already exists, aborted";
        cryptothrow(errormsg.c_str(), -1);
    }
    crypto::write_program_pid(program);

    int logLevel = MonitorConfig::GetInstance().GetLogLevel();
    std::string logPath = MonitorConfig::GetInstance().GetLogPath();

    MonitorOperation* op = new MonitorOperation();
    if(op->preStart()) {
        op->run();
    }
    else {
        cryptothrow("Monitor start failed!", -1);
    }

    while(1) {
        log_maintain(program, logPath, logLevel);
        usleep(1000);
    }

    return 0;
}
