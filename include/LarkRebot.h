#pragma once

#include <string>
#include "Utility.h"


class LarkRebot {
public:
    static LarkRebot& GetInstance();
    ~LarkRebot();
    void SendLarkMsg(const MsgCard& msgCard, const std::string& url, bool atAll = false);
    void SendLarkMsg(const std::string& msg, const std::string& url, bool atAll = false);

private:
    LarkRebot();
};