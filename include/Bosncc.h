/*
bosncc(Based On Serialization network communication Components)
*/

#pragma once
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment( lib, "ws2_32.lib" )

#include "openssl/bio.h"  
#include "openssl/ssl.h"  
#include "openssl/err.h" 
#pragma comment( lib, "libeay32.lib" )
#pragma comment( lib, "ssleay32.lib" )

#include "ThreadTask.h"



#define BOSNCC_MODEL_CLIENT 0x01
#define BOSNCC_MODEL_SERVER 0x02

#define ENABLE_SSL 555
#define DISABLE_SSL 5554

class Bosncc
{
public:
	static Bosncc *GetInstance();
	int SetRunoModel(int Type);

//客户端模式
	//添加任务
	int AddTask(ThreadTask *UThreadTask);
	
//服务器模式
	int Start();
	void SetListenport(unsigned int UPort);
	unsigned int GetListenport();
//SSL加密通讯
	int SetSSLCertificate(std::string UFilePath);
	int SetSSLCertificate(std::string UPublicKey, std::string UPrivateKey,unsigned int KeyLenth);
//业务函数注册
	void SetBusinessFunction(Business_Function_CallBack Uf);

//通用方法
	int Init();
	
	void SetTimeout(unsigned int UTimeout);
	int GetTimeout();

	int SetEnableSSL(unsigned int UValue);
	int GetEnableSSL();

private:
	static int RunModel;
	static bool RunSetModel;

	static bool CreatInstance;
	static Bosncc * m_Instance;

	bool Started;

	Bosncc();
	~Bosncc();

	int ClientInit();
	int ServerInit();
	int SSLInit();
	

	unsigned int Listenport;
	PTP_POOL WindowsThreadPool;
	sockaddr_in Bosncc_Server_addr_in;

	WORD wVersionRequested;
	WSADATA wsaData;

	Business_Function_CallBack Business_CallBack;//业务处理函数

	//加密
	unsigned int EnableSSLConnect;
	bool SSLInitComplete;
	SSL_CTX *ServerCtx;
	
	//超时时间
	unsigned int Timeout;

	evutil_socket_t Bosncc_Socket;
	struct event_base *Main_Base;

	//Accept回调函数
	static void On_Accept_CallBack(evutil_socket_t UESlistenSocket, short USevent, void * UVarg);
	//read 回调函数
	static void On_Read_CallBack(struct bufferevent *Ubev, void *UVarg);
	static void On_Read_CallBack_SSL(struct bufferevent *Ubev, void *UVarg);
	//error回调函数
	static void On_Error_CallBack(struct bufferevent *Ubev, short USevent, void *UVarg);
	//write 回调函数
	static void On_Write_CallBack(struct bufferevent *Ubev, void *UVarg);
	static void On_Write_CallBack_SSL(struct bufferevent *Ubev, void *UVarg);

	//windows线程池处理函数
	static VOID CALLBACK ServerThreadPoolWorking(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work);
	static VOID CALLBACK ClienThreadPoolWorking(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work);
};

