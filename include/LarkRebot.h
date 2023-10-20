#pragma once

#include <cpprest/http_client.h>
#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <string>
#include "Utility.h"

using namespace std;


class LarkRebot {
public:
    static LarkRebot& GetInstance();
    ~LarkRebot();
    void SendLarkMsg(MsgCard& msgCard, string larUrl);
    void SendMsg(string msg, ReceiveInfo& receiveInfo);
    void SendGroupMsg(string msg, ReceiveGroupInfo& receiveGroupInfo);
    void SendGroupMsgCard(MsgCard& msgCard, ReceiveGroupInfo& receiveGroupInfo);
    void SendVoiceCall(string msg, string receiveId);
    void SendGroupVoiceCall(string msg, string groupId, vector<string>& vUserId);
private:
    LarkRebot();
    string url;
    string authorization;
    string groupUrl;
    string voiceCallUrl;
    string groupVoiceCallUrl;
    unordered_map<string, int64_t> mIdTime;
    int channel;
    double voiceCallInterval;
};