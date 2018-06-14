#pragma once

#include <iostream>

#include <WinSock2.h>
#include <Ws2tcpip.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#pragma comment (lib,"libevent.lib")
#pragma comment (lib,"libevent_core.lib")
#pragma comment (lib,"libevent_extras.lib")

#include <protoc/protoc.h>
#pragma comment( lib, "libprotobufd.lib" )
#pragma comment( lib, "libprotocd.lib" )
#pragma comment( lib, "libprotobuf-lited.lib" )
typedef int(*bbbbb)(unsigned int &StatusCode, std::string &ApiName, ParameterGroup &UParameterGroup,char *Data,unsigned int &Lenth);
typedef int(*Business_Function_CallBack)(ConnectProtocol * UConnectProtocol,int Arg);
/*
参数
	UConnectProtocol	序列化对象指针
	Arg					当前通讯状态
		100 发送完成
		300 接收完成
返回值
	1	持续连接
	11	持续接收
	返回2 关闭连接
*/

class ThreadTask
{
public:
	bool DataOperationStatus;

	unsigned int DataLenth_All;
	bool DataLenth_All_Done;
	unsigned int DataLenth_Already;
	bool DataLenth_Already_Done;
	unsigned int DataLenth_Remaining;
	bool DataLenth_Remaining_Done;

	unsigned int Verification;
	bool Verification_Done;
	bool Verification_Ok;
	//加密
	bool Encrypt;
	bool EncryptDone;

	char *DataBuff;

	int DataOperationEnd();
	int DataOperationReady();
public:
	ThreadTask();
	~ThreadTask();

	//序列化协议
	ConnectProtocol m_ConnectProtocol;
	void operator = (ConnectProtocol UConnectProtocol);
	void operator = (ThreadTask UThreadTask);

	//event
	struct event_base *Threadbase;
	evutil_socket_t ConnectSocket;
	struct sockaddr_in ConnectSocketAddr_in;
	Business_Function_CallBack Business_CallBack;

	//线程注册
	int ThreadRegistered();
	int ThreadUnregister();


	//只有注册线程可以调用
	int SetProcessedDone(int Ucode);

	void SetTimeout(unsigned int UTimeout);
	unsigned int GetTimeout();

	void SetTaskMode(unsigned int Umode);
	unsigned int GetTaskMode();

	int SetParameterGroup(ParameterGroup &UParameterGroup);

	//客户端模式使用
	unsigned int Connectport;
	std::string ConnectAddress;

private:

	unsigned int TaskMode;
	//发送  100
	//发送后断开 101
	//接收  300
	//接收后断开 301

	//超时时间
	unsigned int Timeout;


	unsigned long ParentThread;
	unsigned long ChildThreadID;
};
