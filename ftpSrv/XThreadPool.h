#pragma once
#include <vector>

class XThread;
class XTask;
class XThreadPool
{
public:
	// 单例模式
	static XThreadPool *Get() {
		static XThreadPool p;
		return &p;
	}
	// 初始化所有线程
	void Init(int threadCount);

	// 分发线程
	void Dispatch(XTask*);
private:
	int threadCount; // 线程池中创建的线程数
	int lastThread = -1; // 前一个线程编号
	std::vector<XThread *> threads;
	XThreadPool() {};
};

