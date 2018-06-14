#pragma once
#define LYSS_CONNECTCHAR(a,b) a##b
#define ANNOTATION LYSS_CONNECTCHAR(/,/)
#ifdef _DEBUG
	#define TRACE printf
#else
	#define TRACE ANNOTATION
#endif // _DEBUG


#define READY_TO_SEND 21212
#define CAN_SEND_DATA_LENTH 100
#define CAN_SEND_DATA_VERIFY 200
#define CAN_SEND_DATA 202
#define CAN_SEND_DATA_END 203

#define READY_TO_RECV 2123
#define CAN_RECV_DATA_LENTH 300
#define CAN_RECV_DATA_VERIFY 400
#define CAN_RECV_DATA 500
#define CAN_RECV_DATA_END 501

#define CBR_RECV 568732
#define CBR_ALWAYS_RECV 5687
#define CBR_SEND 56817
#define CBR_CLOSE 56827

#define ENABLE_SSL 555
#define DISABLE_SSL 5554

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

typedef int(*Business_Function_CallBack)(ParameterGroup & UParameterGroup,int Arg);
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
	ThreadTask();
	~ThreadTask();

	//全部
	int GetDataCount();
	int SetDataCount(unsigned int USize);
	char* GetDatapointer(unsigned int USize);

	//完成
	int GetDataAlreadyCount();
	int SetDataAlreadyCount(int UMod, unsigned int USize);

	//剩余
	int GetDataRemainingCount();
	int SetDataRemainingCount(int UMod, unsigned int USize);

	//效验
	int SetVerificationValue(unsigned int UValue);
	int GetVerification();
	int GetVerificationValue();

	int GetDataStage();
	int SetDataStage(unsigned int UValue);

	//加密
	int SetEnableSSL(int UMod);
	int GetSSLEnable();
	/*void *GetSSLConversation();*/

	//线程注册
	int ThreadRegistered();
	int ThreadUnregister();

	int DataPreparation();

	int SetProcessedDone(int Ucode);

	void SetTimeout(unsigned int UTimeout);
	unsigned int GetTimeout();

	void SetTaskMode(unsigned int Umode);
	unsigned int GetTaskMode();

	int SetParameterGroup(ParameterGroup &UParameterGroup);
	//int GetParameterGroup();

	void operator = (ConnectProtocol UConnectProtocol);
	void operator = (ThreadTask UThreadTask);

	int SetConnectPort(unsigned int Uport);
	unsigned int GetConnectPort();

	int SetConnectAddress(std::string UAddress);
	std::string GetConnectAddress();

	int Set_BF_Callback(Business_Function_CallBack UCallback);
	int Call_BF_Callback(int Arg);

	//event
	struct event_base *Threadbase;

	evutil_socket_t ConnectSocket;

	struct sockaddr_in ConnectSocketAddr_in;

	struct bufferevent *Bufferevent;

	void *SSLConversation;

private:
	//数据传输处于什么阶段
	int DataStage;
	/*
	10 正在准备发送数据
	20 数据初始化准备完成
	100 准备发送SSL请求
	200	准备发送数据大小
	300 准备发送数据效验
	400 准备在发送数据
	401 正在发送方数据
	402 发送数据结束


	50 数据接收准备
	60 数据接收准备完成
	100 准备接收SSL请求
	600 准备接收数据大小
	700 准备接收数据效验
	701 正在接收数据效验
	702 数据效验接收完成
	800 准备接收数据
	801	正在接收数据
	802	接收数据完成
	*/

	//全部数据计数
	unsigned int DataLenth_Count;

	//完成
	unsigned int DataLenth_Already_Count;

	//剩余
	unsigned int DataLenth_Remaining_Count;

	//效验
	unsigned int Verification;

	//加密
	int EnableSSLConnect;

	//缓冲区
	char *DataBuff;

	//序列化协议
	ConnectProtocol m_ConnectProtocol;
	ParameterGroup m_ParameterGroup;

	Business_Function_CallBack Business_CallBack;

	//客户端模式使用
	unsigned int Connectport;
	std::string ConnectAddress;

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
