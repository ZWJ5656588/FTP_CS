#include "XFtpFactory.h"
#include "XFtpServerCMD.h"
#include "XFtpUSER.h"
#include "XFtpLIST.h"
#include "XFtpPORT.h"
#include "XFtpRETR.h"
#include "XFtpSTOR.h"
#include "testUtil.h"
#include "XFtpDELE.h"
//#include "XFtpPASIV.h"
 
 // 工厂设计模式
XTask *XFtpFactory::CreateTask() {
	testout("At XFtpFactory::CreateTask");
	XFtpServerCMD *x = new XFtpServerCMD();
	
	// 

	x->Reg("USER", new XFtpUSER()); //USER 命令用于客户端发送用户名给服务器

	x->Reg("PORT", new XFtpPORT()); //PORT主动模式 客户端开放大于1024的端口 服务器连接

	//x->Reg("PASIV",new XFtpPASIV()); //PASIV被动模式 服务器开放大于1024的端口让客户端连接

	XFtpTask *list = new XFtpLIST();
	x->Reg("PWD", list);  // 显示当前工作目录
	x->Reg("LIST", list); // 切换父目录
	x->Reg("CWD", list); // 改变工作目录
	x->Reg("CDUP", list); // 切换父目录

	x->Reg("RETR", new XFtpRETR()); // 下载文件

	x->Reg("STOR", new XFtpSTOR()); // 上传文件
	
	return x;
}

XFtpFactory::XFtpFactory() {

}