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
	ThreadTask();
	~ThreadTask();

	//ȫ��
	int GetDataCount();
	int SetDataCount(unsigned int USize);
	char* GetDatapointer(unsigned int USize);

	//���
	int GetDataAlreadyCount();
	int SetDataAlreadyCount(int UMod, unsigned int USize);

	//ʣ��
	int GetDataRemainingCount();
	int SetDataRemainingCount(int UMod, unsigned int USize);

	//Ч��
	int SetVerificationValue(unsigned int UValue);
	int GetVerification();
	int GetVerificationValue();

	int GetDataStage();
	int SetDataStage(unsigned int UValue);

	//����
	int SetEnableSSL(int UMod);
	int GetSSLEnable();
	/*void *GetSSLConversation();*/

	//�߳�ע��
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
	//���ݴ��䴦��ʲô�׶�
	int DataStage;
	/*
	10 ����׼����������
	20 ���ݳ�ʼ��׼�����
	100 ׼������SSL����
	200	׼���������ݴ�С
	300 ׼����������Ч��
	400 ׼���ڷ�������
	401 ���ڷ��ͷ�����
	402 �������ݽ���


	50 ���ݽ���׼��
	60 ���ݽ���׼�����
	100 ׼������SSL����
	600 ׼���������ݴ�С
	700 ׼����������Ч��
	701 ���ڽ�������Ч��
	702 ����Ч��������
	800 ׼����������
	801	���ڽ�������
	802	�����������
	*/

	//ȫ�����ݼ���
	unsigned int DataLenth_Count;

	//���
	unsigned int DataLenth_Already_Count;

	//ʣ��
	unsigned int DataLenth_Remaining_Count;

	//Ч��
	unsigned int Verification;

	//����
	int EnableSSLConnect;

	//������
	char *DataBuff;

	//���л�Э��
	ConnectProtocol m_ConnectProtocol;
	ParameterGroup m_ParameterGroup;

	Business_Function_CallBack Business_CallBack;

	//�ͻ���ģʽʹ��
	unsigned int Connectport;
	std::string ConnectAddress;

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
