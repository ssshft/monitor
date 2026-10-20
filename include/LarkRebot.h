#pragma once

#include <string>
#include "Utility.h"


class LarkRebot {
public:
    static LarkRebot& GetInstance();
    ~LarkRebot();
    void SendLarkMsg(const MsgCard& msgCard, const std::string& url);
    void SendLarkMsg(const std::string& msg, const std::string& url);

private:
    LarkRebot();
};