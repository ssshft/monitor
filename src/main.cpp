#include "MonitorConfig.h"
#include "program_util.h"
#include "crypto_exception.h"
#include "MonitorOperation.h"

#include "LarkRebot.h"

std::mutex mut;


// to do
// 1. 账户持仓付出的资金费用需要报警
int main(int argc, char* argv[]) {
    LarkRebot::GetInstance().SendLarkMsg("test!", "https://open.feishu.cn/open-apis/bot/v2/hook/6fb9a793-8a4b-44ff-8247-5633a182cc93");
    return 0;


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

    std::string logLevel = std::to_string(MonitorConfig::GetInstance().GetLogLevel());
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
