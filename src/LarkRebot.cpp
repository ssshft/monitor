#include "LarkRebot.h"
#include "MonitorConfig.h"
#include "Net.h"

LarkRebot::LarkRebot() {
}

LarkRebot::~LarkRebot() {
}

LarkRebot& LarkRebot::GetInstance() {
	static LarkRebot larkRebot;
	return larkRebot;
}

void LarkRebot::SendLarkMsg(const MsgCard& msgCard, const std::string& url) {
    std::string targetUrl = url;
	size_t scheme_pos = url.find("://");
	if (scheme_pos != std::string::npos) {
		targetUrl = url.substr(scheme_pos + 3);
	}

	// 查找第一个 /
	size_t slash_pos = targetUrl.find('/');
	std::string host = targetUrl.substr(0, slash_pos);
	std::string target = targetUrl.substr(slash_pos);
	string msg = msgCard.object + "\\n" + msgCard.datetime + "\\n" + msgCard.content;
	
	std::string body_in = fmt::format("{{\"msg_type\":\"text\",\"content\":{{\"text\":\"{}\"}}}}", msg);

   	int status = 0;
    std::string body;
    try {
        if (!Net::Instance().syncPost("lark", host, target, body_in, {}, {}, body, status)) {
            LOG_ERROR("SendLarkMsg syncPost return false");
            return;
        }
        if (status != 200) {
            LOG_INFO("SendLarkMsg status: {}", status);
            return;
        }
    }
    catch(exception& e) {
        LOG_INFO("SendLarkMsg Error: {}", e.what());
    }
}

void LarkRebot::SendLarkMsg(const std::string& msg, const std::string& url) {
    std::string targetUrl = url;
	size_t scheme_pos = url.find("://");
	if (scheme_pos != std::string::npos) {
		targetUrl = url.substr(scheme_pos + 3);
	}

	// 查找第一个 /
	size_t slash_pos = targetUrl.find('/');
	std::string host = targetUrl.substr(0, slash_pos);
	std::string target = targetUrl.substr(slash_pos);
	
	std::string body_in = fmt::format("{{\"msg_type\":\"text\",\"content\":{{\"text\":\"{}\"}}}}", msg);

   	int status = 0;
    std::string body;
    try {
        if (!Net::Instance().syncPost("lark", host, target, body_in, {}, {}, body, status)) {
            LOG_ERROR("SendLarkMsg syncPost return false");
            return;
        }
        if (status != 200) {
            LOG_INFO("SendLarkMsg status: {}", status);
            return;
        }
    }
    catch(exception& e) {
        LOG_INFO("SendLarkMsg Error: {}", e.what());
    }
}
