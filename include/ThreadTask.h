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
����
	UConnectProtocol	���л�����ָ��
	Arg					��ǰͨѶ״̬
		100 �������
		300 �������
����ֵ
	1	��������
	11	��������
	����2 �ر�����
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
	//����
	bool Encrypt;
	bool EncryptDone;

	char *DataBuff;

	int DataOperationEnd();
	int DataOperationReady();
public:
	ThreadTask();
	~ThreadTask();

	//���л�Э��
	ConnectProtocol m_ConnectProtocol;
	void operator = (ConnectProtocol UConnectProtocol);
	void operator = (ThreadTask UThreadTask);

	//event
	struct event_base *Threadbase;
	evutil_socket_t ConnectSocket;
	struct sockaddr_in ConnectSocketAddr_in;
	Business_Function_CallBack Business_CallBack;

	//�߳�ע��
	int ThreadRegistered();
	int ThreadUnregister();


	//ֻ��ע���߳̿��Ե���
	int SetProcessedDone(int Ucode);

	void SetTimeout(unsigned int UTimeout);
	unsigned int GetTimeout();

	void SetTaskMode(unsigned int Umode);
	unsigned int GetTaskMode();

	int SetParameterGroup(ParameterGroup &UParameterGroup);

	//�ͻ���ģʽʹ��
	unsigned int Connectport;
	std::string ConnectAddress;

private:

	unsigned int TaskMode;
	//����  100
	//���ͺ�Ͽ� 101
	//����  300
	//���պ�Ͽ� 301

	//��ʱʱ��
	unsigned int Timeout;


	unsigned long ParentThread;
	unsigned long ChildThreadID;
};
