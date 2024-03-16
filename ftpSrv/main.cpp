#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::cerr;
using std::flush;
#include <event2/event.h>
#include <stdlib.h>
 
#include <signal.h>
 
#define errmsg(msg) do{cout<< msg << endl;exit(1);}while(0)
/**************basic include***************/
#include <string.h>
#include <string>
using std::string;
#include <fstream>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include "testUtil.h"
#include "XThreadPool.h"
#include "XThread.h"
#include "XTask.h"
#include "XFtpFactory.h"


#define SPORT 21
#define BUFS 10240

#define XThreadPoolGet XThreadPool::Get()

/*
 *  接待连接的回调函数！！！
 */
void listen_cb(struct evconnlistener *ev, evutil_socket_t s, struct sockaddr *addr, int socklen, void *arg) {
	printf("%s <%s:%d> pid = %d, tid = %ld ",__FILE__,__FUNCTION__,__LINE__,getpid(), pthread_self());
	sockaddr_in *sin = (sockaddr_in*)addr;
	/*
     *  创建任务
     */
	// 动态绑定 以及工厂模式
	XTask *task = XFtpFactory::Get()->CreateTask(); // 使用时候创建单例
	if(task == nullptr)
	{
		printf("%s <%s:%d> pid = %d, tid = %ld ret = %d create task error!",__FILE__,__FUNCTION__,__LINE__,getpid(),pthread_self());
	} 
	task->sock = s; // 这里就是连接的回调完成了和客户端的三次握手 拿到客户端的信息 赋值给XFtp的任务类 方便任务类使用buffevent与客户端进行通信
	/*
     *  分配任务
     */
	XThreadPoolGet->Dispatch(task); // 创建现成池对象并分配任务
}

void null_log_callback(int severity, const char *msg) {
    // 这个函数体为空，因此所有 libevent 的日志都会被忽略。
}

int main() {

	event_set_log_callback(null_log_callback);
 
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        errmsg("signal(SIGPIPE, SIG_IGN) error!");
 

    /*
     *  初始化线程池
     */
	XThreadPoolGet->Init(30);

	event_base *base = event_base_new();
	if (!base)
		printf("%s <%s:%d> main thread event_base_new error",__FILE__,__FUNCTION__,__LINE__);

    /*
     *  创建libevent上下文
     */
	sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SPORT);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
    /*
     *  创建监听事件
     */

	// evconnlisener维护创建出来的套接字
	evconnlistener *ev = evconnlistener_new_bind(
		base,											// libevent的上下文
		listen_cb,										//接收到连接的回调函数
		base,											//地基
		LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,		//地址重用，evconnlistener关闭同时关闭socket
		10000,											//连接队列大小，对应listen函数
		(sockaddr*)&sin,								//绑定的地址和端口
		sizeof(sin));  // 这是服务端的地址信息
	
	cout << "ev:" << ev << endl;
	cout << "err:" << errno << endl;
	if (base) {
		int ret = event_base_dispatch(base); // 进入事件循环 等待响应事件 while epoll_wait
		printf("%s <%s:%d> pid = %d, tid = %ld ret = %d",__FILE__,__FUNCTION__,__LINE__,getpid(),pthread_self(),ret);
	}
	cout << "event_base_dispatch end..." << endl;
	if (ev)
		evconnlistener_free(ev);
	if (base)
		event_base_free(base);
	testout("server end");
	return 0;
}

