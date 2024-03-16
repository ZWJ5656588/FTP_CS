#pragma once
#include "XTask.h"


class XFtpFactory
{
public:
	static XFtpFactory*Get() {
		static XFtpFactory f; // 懒汉式  C++11保证了局部静态变量的初始化是线程安全的
		return &f;
	}
	XTask *CreateTask();
private:
	XFtpFactory();
};

/*

饿汉式

class XFtpFactory
{
public:
	static XFtpFactory* Get()
	{
		return &instance;
	}
	XTask *CreateTask();
}

private:
	XFtpFactory() {} // 私有构造
	static XFtpFactory intance; // 程序开始时候就创建了单例 Get的也是instance;

*/

