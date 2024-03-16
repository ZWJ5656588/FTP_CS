#include <string.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/util.h>

#include <string>
using namespace std;

#include "XFtpServerCMD.h"
#include "testUtil.h"

#define BUFS 4096

// 注册ftp命令 
void XFtpServerCMD::Reg(std::string cmd, XFtpTask *call) {
	testout("At XFtpServerCMD::Reg");
	if (!call) {
		cout << "XFtpServerCMD::Reg call is null " << endl;
		return;
	}
	if (cmd.empty()) {
		cout << "XFtpServerCMD::Reg cmd is null " << endl;
		return;
	}
	// 已经注册的是否覆盖，不覆盖，提示错误
	if (calls.find(cmd) != calls.end()) {
		cout << cmd << " is alredy register" << endl;
		return;
	}

	// 命令处理对象与命令字符串关联并且存储在calls映射中
	testout(cmd << " Reg success");
	call->base = base; // call是ftp任务对象 calls将任务对象与string关联
	call->cmdTask = this;
	calls[cmd] = call;
	calls_del[call] = 0;
}

void XFtpServerCMD::Event(bufferevent *bev, short events) {
	testout("At XFtpServerCMD::Event");
	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) {
		delete this;
	}
}

void XFtpServerCMD::Read(bufferevent *bev) { // 重写了基类的 Read 函数，用于处理从客户端接收到的数据。这里的处理通常指解析FTP命令并执行相应的操作。
	cout << endl;
	testout("At XFtpServerCMD::Read");
	char buf[BUFS] = { 0 };
	while (1) {
		int len = bufferevent_read(bev, buf, BUFS); //异步读 出发读回调
		if (len <= 0) break;
		cout << "Recv CMD(" << len << "):" << buf;
		// 分发到处理对象
		// 分析出类型
		string type = "";
		for (int i = 0; i < len; i++) {
			if (buf[i] == ' ' || buf[i] == '\r')
				break;
			type += buf[i];
		}

		// 解析命令类型并调用命令对象进行解析
		cout << "type is [" << type << "]" << endl;
		if (calls.find(type) != calls.end()) {
			testout("begin to parse");
			XFtpTask *t = calls[type];
			t->Parse(type, buf);
			testout("curDir: [" << curDir << "]");
		}
		else {
			cout << "parse object not found" << endl;
			ResCMD("200 OK\r\n");
		}
	}
}

bool XFtpServerCMD::Init() { // 在调用lisenercb之后创建一个bufferevent 一旦客户端通信 
	testout("At XFtpServerCMD::Init");

	// 在线程的base里添加一个缓冲区对sock的缓冲事件，这就是命令通道
	// bufferevent 处理异步读写 将输入输出缓冲区传给内核
	bufferevent *bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE); // sock是XTask的通信文件描述符
	if (!bev) {
		delete this;
		return false;
	}

	// 添加超时
	timeval t = {300, 0};
	bufferevent_set_timeouts(bev, &t, 0); // 设置读写超时

	string msg = "220 Welcome to XFtpServer\r\n";
	bufferevent_write(bev, msg.c_str(), msg.size()); // 先向客户端写

	this->cmdTask = this;  // 绑定bufferevent对象
	this->bev = bev;
	// 注册本对象实现的回调函数
	Setcb(bev);

	return true;
}

XFtpServerCMD::XFtpServerCMD() {
}

XFtpServerCMD::~XFtpServerCMD() {
	ClosePORT();
	for (auto i : calls_del) {
		delete i.first;
	}
}
