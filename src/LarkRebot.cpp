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

std::string LarkRebot::JsoneEcaped(const std::string& msg) {     // 需要先对 msg 做 JSON 字符串转义
    std::string escaped = "";
    for (char c : msg) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '"':  escaped += "\\\""; break;
            case '\n': escaped += "\\n";  break;
            case '\r': escaped += "\\r";  break;
            case '\t': escaped += "\\t";  break;
            default:   escaped += c;      break;
        }
    }

    return escaped;
}

void LarkRebot::SendLarkMsg(const MsgCard& msgCard, const std::string& url, bool atAll) {
    std::string targetUrl = url;
	size_t scheme_pos = url.find("://");
	if (scheme_pos != std::string::npos) {
		targetUrl = url.substr(scheme_pos + 3);
	}

	// 查找第一个 /
	size_t slash_pos = targetUrl.find('/');
	std::string host = targetUrl.substr(0, slash_pos);
	std::string target = targetUrl.substr(slash_pos);
	string msg = msgCard.object + "\n" + msgCard.datetime + "\n" + msgCard.content;

    if (atAll) {
        msg = "<at user_id=\"all\">所有人</at>" + msg;
    }

    std::string text = JsoneEcaped(msg);

	std::string body_in = fmt::format("{{\"msg_type\":\"text\",\"content\":{{\"text\":\"{}\"}}}}", text);

    LOG_INFO("SendLarkMsg body_in: {}", body_in);

   	int status = 0;
    std::string body;
    try {
        if (!Net::Instance().syncPost("lark", host, target, body_in, {}, {}, body, status)) {
            LOG_ERROR("SendLarkMsg syncPost return false");
            return;
        }
        if (status != 200) {
            LOG_INFO("SendLarkMsg status: {} body: {}", status, body);
            return;
        }
    }
    catch(exception& e) {
        LOG_INFO("SendLarkMsg Error: {}", e.what());
    }
}

void LarkRebot::SendLarkMsg(const std::string& msg, const std::string& url, bool atAll) {
    std::string targetUrl = url;
	size_t scheme_pos = url.find("://");
	if (scheme_pos != std::string::npos) {
		targetUrl = url.substr(scheme_pos + 3);
	}

	// 查找第一个 /
	size_t slash_pos = targetUrl.find('/');
	std::string host = targetUrl.substr(0, slash_pos);
	std::string target = targetUrl.substr(slash_pos);

    std::string s = msg;
    if (atAll) {
        s = "<at user_id=\"all\">所有人</at>" + msg;
    }

    std::string text = JsoneEcaped(s);

	std::string body_in = fmt::format("{{\"msg_type\":\"text\",\"content\":{{\"text\":\"{}\"}}}}", text);

   	int status = 0;
    std::string body;
    try {
        if (!Net::Instance().syncPost("lark", host, target, body_in, {}, {}, body, status)) {
            LOG_ERROR("SendLarkMsg syncPost return false");
            return;
        }
        if (status != 200) {
            LOG_INFO("SendLarkMsg status: {} body: {}", status, body);
            return;
        }
    }
    catch(exception& e) {
        LOG_INFO("SendLarkMsg Error: {}", e.what());
    }
}
