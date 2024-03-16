#include <thread>
#include <iostream>
using namespace std;
 
#include <unistd.h>
 
#include <event2/event.h>

#include "testUtil.h"
#include "XThread.h"
#include "XTask.h"

/*这是一个利用libevent的线程类
线程间通过管道进行通信，主线程可以向子线程发送激活信号，并分配任务给子线程处理。这
种设计模式适用于需要处理大量异步IO操作的应用程序，*/


// 静态回调 被注册到event_base中
static void Notify_cb(evutil_socket_t fd, short which, void *arg) {
	XThread *t = (XThread*)arg;
	t->Notify(fd, which);
}

// 处理发送来的通知信号 读取到信号时进行操作
void XThread::Notify(evutil_socket_t fd, short which) {
	testout(id << " thread At Notify()");
	char buf[2] = { 0 };
 
	int re = read(fd, buf, 1);
	if (re < 0)
		return;
	cout << id << " thread recv" << buf << endl;
}


/*
 *  开启线程
 */
void XThread::Start() {
	testout(id << " thread At Start()");
	Setup(); // 进行线程初始化
	thread th(&XThread::Main, this); // 创建线程并执行线程函数
	th.detach(); // 线程分离
}

/*
 *  线程主函数  每一个线程有一个事件循环 
 */
void XThread::Main() {
	cout << id << " thread::Main() begin" << endl;
	event_base_dispatch(base); // 启动事件循环 跨线程通信
	event_base_free(base);
	cout << id << " thread::Main() end" << endl;
}

/*
 *  初始化线程
 */
bool XThread::Setup() {
	testout(id << " thread At Setup");

	int fds[2]; // 使用管道进行线程间通信

	if (pipe(fds)) {
		cerr << "pipe failed" << endl;
		return false;
	}

	// 将管道的写端赋值到notify_send_fd成员变量
	notify_send_fd = fds[1];

	// // 设置event_base不使用锁，因为它只在本线程中使用
	event_config *ev_conf = event_config_new();
	event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK);
	this->base = event_base_new_with_config(ev_conf); // 根据配置创建一个event_base
	event_config_free(ev_conf); // 释放event_config对象
	if (!base) {
		cout << "event_base_new_with_config error!" << endl;
		return false;
	}

	// 创建一个事件循环 当fds[0]可读时候出发Notify_cb回调
	event *ev = event_new(base, fds[0], EV_READ | EV_PERSIST, Notify_cb, this);
	event_add(ev, 0); // 将事件添加到事件循环

	return true;
}

/*
 *  激活线程 管道发送一个信号让事件循环中相应的事件触发 
 */
void XThread::Activate() {
	testout(id << " thread At Activate()");

	// 管道写端写入数据时 读端会通过libevent进行回调
	int re = write(notify_send_fd, "c", 1); // 向管道的写端写入数据 会激活等待在读端的事件

	if (re <= 0) {
		cerr << "XThread::Activate() fail" << endl;
	}
	// 互斥锁用于锁定任务队列
	XTask *t = NULL;
	tasks_mutex.lock();
	if (tasks.empty()) {
		tasks_mutex.unlock();
		return;
	}
	t = tasks.front();
	tasks.pop_front();
	tasks_mutex.unlock();
	t->Init(); // 初始化任务 触发bufferevent向客户端写
}

/*
 *  添加任务
 */
void XThread::AddTack(XTask *t) {
	if (!t) return;

	// 将当前线程的事件地基赋值给任务 使得任务可以在当前线程的事件循环中处理
	t->base = this->base; 

	tasks_mutex.lock();
	tasks.push_back(t); // 添加任务队列
	tasks_mutex.unlock();
}

XThread::XThread() {

}
XThread::~XThread() {

}
