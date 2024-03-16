#include "XFtpTask.h"
#include "testUtil.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <string.h>
#include <iostream>
using namespace std;

// 连接到客户端指定的PORT模式地址
void XFtpTask::ConnectoPORT() {
    testout("At XFtpTask::ConnectoPORT");
    // 检查命令任务的IP地址和端口是否有效
    if (cmdTask->ip.empty() || cmdTask->port <= 0 || !cmdTask->base) {
        cout << "ConnectPORT failed" << endl;
        return;
    }
    // 如果已经存在一个bufferevent对象，先释放它
    if (bev) {
        bufferevent_free(bev);
        bev = 0;
    }
    // 创建一个新的bufferevent对象用于网络通信
    bev = bufferevent_socket_new(cmdTask->base, -1, BEV_OPT_CLOSE_ON_FREE);
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(cmdTask->port);
    evutil_inet_pton(AF_INET, cmdTask->ip.c_str(), &sin.sin_addr.s_addr);

    // 设置bufferevent的回调函数
    Setcb(bev); 

    // 设置连接超时时间为60秒
    timeval t = {60, 0};
    bufferevent_set_timeouts(bev, &t, 0);

    // 开始连接到指定的客户端地址
    bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
}

// 关闭PORT模式连接
void XFtpTask::ClosePORT() {
    // 释放bufferevent对象
    if (bev) {
        bufferevent_free(bev);
        bev = 0;
    }
    // 如果有打开的文件指针，关闭它
    if (fp) {
        fclose(fp);
        fp = 0;
    }
}

// 设置bufferevent的回调函数 一个fd 两个缓冲区 三个回调
void XFtpTask::Setcb(bufferevent *bev) {
    // 设置读、写、事件回调，传入this作为回调的上下文
    bufferevent_setcb(bev, ReadCB, WriteCB, EventCB, this);
    // 启用读写事件监听
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

// 发送数据（string重载）
void XFtpTask::Send(const string &data) {
    testout("At XFtpTask::Send");
    Send(data.c_str(), data.size());
}

// 发送数据（char*重载）
void XFtpTask::Send(const char *data, size_t datasize) {
    testout("At XFtpTask::Send");
    cout << data;
    cout << datasize << endl;
    if (datasize == 0) return;
    // 如果bufferevent对象存在，通过它发送数据
    if (bev) {
        bufferevent_write(bev, data, datasize);
    }
}

// 响应FTP命令
void XFtpTask::ResCMD(string msg) {
    testout("At XFtpTask::ResCMD");
    if (!cmdTask || !cmdTask->bev) return;
    cout << "ResCMD: " << msg << endl << flush;
    // 确保消息以"\r\n"结尾
    if (msg[msg.size() - 1] != '\n') {
        msg += "\r\n";
    }
    // 通过命令任务的bufferevent发送响应
    bufferevent_write(cmdTask->bev, msg.c_str(), msg.size());
}

// 事件回调函数
void XFtpTask::EventCB(bufferevent *bev, short events, void *arg) {
    XFtpTask *t = (XFtpTask*)arg;
    t->Event(bev, events);
}

// 读取回调函数
void XFtpTask::ReadCB(bufferevent *bev, void *arg) {
    XFtpTask *t = (XFtpTask*)arg;
    t->Read(bev);
}

// 写入回调函数
void XFtpTask::WriteCB(bufferevent *bev, void *arg)
{
	XFtpTask *t = (XFtpTask*)arg;
    t->Write(bev);
}

// 析构函数，在对象销毁时调用，用于清理资源
XFtpTask::~XFtpTask() {
    ClosePORT(); // 关闭PORT模式连接和相关资源
}