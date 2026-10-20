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

void LarkRebot::SendLarkMsg(MsgCard& msgCard, std::string url) {
	// https://open.feishu.cn/open-apis/bot/v2/hook/6fb9a793-8a4b-44ff-8247-5633a182cc93

	size_t scheme_pos = url.find("://");
	if (scheme_pos != std::string::npos) {
		url = url.substr(scheme_pos + 3);
	}

	// 查找第一个 /
	size_t slash_pos = url.find('/');
	std::string host = url.substr(0, slash_pos);
	std::string target = url.substr(slash_pos);
	string msg = msgCard.object + " " + msgCard.datetime + " " + msgCard.content;
	
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
