#pragma once
class XTask
{
public:
	// 一客户端一个base
	struct event_base *base = 0; // 这个base在创建线程之后

	// 连接的sock
	int sock = 0; // 这里是客户端的通信套接字

	// 线程池id
	int thread_id = 0;

	// 初始化任务
	virtual bool Init() = 0;
};

