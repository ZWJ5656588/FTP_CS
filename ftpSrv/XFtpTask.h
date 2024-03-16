#pragma once
#include <event2/bufferevent.h>

#include "XTask.h"

#include <string>
using namespace std;

struct bufferevent;

class XFtpTask :
	public XTask
{
public:
	string curDir = "/";
	string rootDir = "/"; // 根目录
	string ip = ""; // 连接的ip地址
	int port = 0; //连接的端口号
	XFtpTask *cmdTask = 0; // 指向管理命令的XFtpTask的实例


	virtual void Parse(std::string, std::string) {}


	void ResCMD(string msg); // 发送命令响应


	void ConnectoPORT();

	void ClosePORT();

	void Send(const string& data);
	void Send(const char *data, size_t datasize);

	virtual void Event(bufferevent *, short) {}
	virtual void Read(bufferevent *) {}
	virtual void Write(bufferevent *) {}

	void Setcb(struct bufferevent*);
	bool Init() { return true; }

	~XFtpTask();

protected:
	static void EventCB(bufferevent *, short, void *);
	static void ReadCB(bufferevent *, void *);
	static void WriteCB(bufferevent *, void *);

	bufferevent *bev = 0;
	FILE *fp = 0;
};

