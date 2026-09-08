#pragma once

#include <string>
#include "Utility.h"


class LarkRebot {
public:
    static LarkRebot& GetInstance();
    ~LarkRebot();
    void SendLarkMsg(MsgCard& msgCard, std::string url);

private:
    LarkRebot();
};