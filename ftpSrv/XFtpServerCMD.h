#pragma once
#include "XFtpTask.h"

#include <map>

class XFtpServerCMD : public XFtpTask
{
public:
	// 初始化任务
	virtual bool Init(); 
	
	// 处理bufferevent相关事件
	virtual void Event(bufferevent *be, short events);

	// 读取客户端发送的数据
	virtual void Read(bufferevent *be);

	// 注册命令处理对象，不需要考虑线程安全，调用时未分发到线程！！
	// 这个函数用于将指定的FTP命令（cmd）与一个处理该命令的 XFtpTask 对象（call）关联起来。这样当接收到该命令时，可以找到并调用相应的处理对象。

	void Reg(std::string, XFtpTask *call);


	XFtpServerCMD();
	~XFtpServerCMD();
private:
	// 命令到处理对象的映射
	std::map<std::string, XFtpTask*>calls;
	// 需要删除的命令处理对象映射
	std::map<XFtpTask*, int>calls_del;
};

