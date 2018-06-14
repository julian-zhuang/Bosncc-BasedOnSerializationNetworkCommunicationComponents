#include "Bosncc.h"

bool Bosncc::CreatInstance = false;
bool Bosncc::RunSetModel = false;
int Bosncc::RunModel = 0;
Bosncc * Bosncc::m_Instance = NULL;

Bosncc * Bosncc::GetInstance()
{
	if (m_Instance == NULL && CreatInstance == false) {
		m_Instance = new Bosncc;
	}
	return m_Instance;
}

int Bosncc::SetRunoModel(int Type)
{
	if (RunSetModel == false && Type == BOSNCC_MODEL_CLIENT) {
		RunModel = BOSNCC_MODEL_CLIENT;
		RunSetModel = true;
		return RunModel;
	}
	if (RunSetModel == false && Type == BOSNCC_MODEL_SERVER) {
		RunModel = BOSNCC_MODEL_SERVER;
		RunSetModel = true;
		return RunModel;
	}
	return -1;
}

int Bosncc::AddTask(ThreadTask * UThreadTask)
{
	if (RunModel != BOSNCC_MODEL_CLIENT) {
		return -1;
	}
	//����DNS��windows��
	struct addrinfo *result = NULL;
	DWORD dwRetval = getaddrinfo(UThreadTask->ConnectAddress.c_str(),0,0,&result);
	if (dwRetval != 0) {
		return -2;
	}
	if (result->ai_family == AF_INET) {
		UThreadTask->ConnectAddress = inet_ntoa(((struct sockaddr_in *) result->ai_addr)->sin_addr);
		UThreadTask->ConnectSocketAddr_in.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(((struct sockaddr_in *)result->ai_addr)->sin_addr));
		UThreadTask->ConnectSocketAddr_in.sin_family = result->ai_family;
	}
	else {
		return -3;
	}
	// ��ʼ����ɣ�����һ��TCP��socket  
	UThreadTask->ConnectSocket = socket(UThreadTask->ConnectSocketAddr_in.sin_family, SOCK_STREAM, IPPROTO_TCP);
	if (UThreadTask->ConnectSocket == INVALID_SOCKET)
	{
		return -4;
	}
	//ָ�����ӵķ������Ϣ  
	UThreadTask->ConnectSocketAddr_in.sin_port = htons(UThreadTask->Connectport);
	 inet_addr(UThreadTask->ConnectAddress.c_str());

	// ������Bind �ͻ����ǽ�������  
	int ret = connect(UThreadTask->ConnectSocket, (SOCKADDR*)&(UThreadTask->ConnectSocketAddr_in), sizeof(SOCKADDR));//��ʼ����  
	if (ret == SOCKET_ERROR)
	{
		closesocket(UThreadTask->ConnectSocket);
		return -5;
	}
	UThreadTask->SetTaskMode(100);
	//�����߳�
	PTP_WORK m_Work = CreateThreadpoolWork(ServerThreadPoolWorking, UThreadTask, NULL);
	SubmitThreadpoolWork(m_Work);
	return 0;
}

int Bosncc::ClientInit()
{
	if (RunModel != BOSNCC_MODEL_CLIENT) {
		return -1;
	}
	int iResult;
	wVersionRequested = MAKEWORD(2, 2);
	iResult = WSAStartup(wVersionRequested, &wsaData);

	if (iResult != 0) {
		return -2;
	}
	else {
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		return -3;
	}

	WindowsThreadPool = CreateThreadpool(NULL);

	if (WindowsThreadPool == NULL) {
		return -4;
	}

	return 0;
}

int Bosncc::ServerInit()
{
	if (RunModel != BOSNCC_MODEL_SERVER) {
		return -1;
	}
	int iResult;
	wVersionRequested = MAKEWORD(2, 2);
	iResult = WSAStartup(wVersionRequested, &wsaData);
	if (iResult != 0) {
		return -2;
	}
	else {
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		return -3;
	}
	Bosncc_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (Bosncc_Socket == INVALID_SOCKET) {
		return -4;
	}
	else {
	}

	Bosncc_Server_addr_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	Bosncc_Server_addr_in.sin_family = AF_INET;
	Bosncc_Server_addr_in.sin_port = htons(Listenport);

	iResult = bind(Bosncc_Socket, (SOCKADDR*)&Bosncc_Server_addr_in, sizeof(SOCKADDR));

	if (iResult == SOCKET_ERROR) {
		closesocket(Bosncc_Socket);
		WSACleanup();
		return -5;
	}
	else {
	}

	iResult = listen(Bosncc_Socket, 5);
	if (iResult == SOCKET_ERROR) {
		return -6;
	}
	else {
	}
	

	WindowsThreadPool = CreateThreadpool(NULL);

	if (WindowsThreadPool == NULL) {
		return -7;
	}

	//�¼�ע��
	evutil_make_socket_nonblocking(Bosncc_Socket);//����socket������ģʽ
	evutil_make_listen_socket_reuseable(Bosncc_Socket);//���õ�ַ����

	Main_Base = event_base_new();
	if (Main_Base == NULL) {
		return -8;
	}

	/*
	����socket�Ŀɶ��¼���������¿ͻ��˵���socket�����Ϊ�ɶ�״̬���ڿɶ���ʱ�򴥷��¼�����������Ӧ�Ļص�����
	*/
	struct event *listen_event = event_new(Main_Base, Bosncc_Socket, EV_READ | EV_PERSIST, On_Accept_CallBack, (void*)Main_Base);
	event_add(listen_event, NULL);
	return 0;
}

int Bosncc::Start()
{
	if (RunModel != BOSNCC_MODEL_SERVER) {
		return -1;
	}
	if (Started) {
		return -2;
	}
	//��ʼ�¼�ѭ��
	Started = true;
	event_base_dispatch(Main_Base);
	Started = false;
	event_base_free(Main_Base);
	return 0;
}

int Bosncc::Init()
{
	if (RunModel == BOSNCC_MODEL_SERVER) {
		return ServerInit();
	}
	if (RunModel == BOSNCC_MODEL_CLIENT) {
		return ClientInit();
	}
	return -444;
}

void Bosncc::SetListenport(unsigned int UPort)
{
	if (Started) {
		return;
	}
	if (RunModel != BOSNCC_MODEL_SERVER) {
		return;
	}
	Listenport = UPort;
}

unsigned int Bosncc::GetListenport()
{
	if (RunModel != BOSNCC_MODEL_SERVER) {
		return 0;
	}
	return Listenport;
}

void Bosncc::SetBusinessFunction(Business_Function_CallBack Uf)
{
	if (RunModel != BOSNCC_MODEL_SERVER) {
		return;
	}
	if (Started) {
		return;
	}
	Business_CallBack = Uf;
}

void Bosncc::SetTimeout(unsigned int UTimeout)
{
	Timeout = UTimeout;
}

int Bosncc::GetTimeout()
{
	return Timeout;
}

void Bosncc::SetEncrypt(bool UEncrypt)
{
	Encrypt = UEncrypt;
}

bool Bosncc::GetEncrypt()
{
	return Encrypt;
}

Bosncc::Bosncc()
{
	Started = false;
	CreatInstance = true;
}


Bosncc::~Bosncc()
{
}

void Bosncc::On_Accept_CallBack(evutil_socket_t UESlistenSocket, short USevent, void * UVarg)
{
	if (RunModel != BOSNCC_MODEL_SERVER) {
		return;
	}
	if (!CreatInstance) {
		return;
	}

	ThreadTask *m_ThreadTask = new ThreadTask;

	m_ThreadTask->SetTaskMode(300);
	m_ThreadTask->Encrypt = Bosncc::GetInstance()->Encrypt;
	m_ThreadTask->SetTimeout(Bosncc::GetInstance()->Timeout);
	
	m_ThreadTask->Business_CallBack = Bosncc::GetInstance()->Business_CallBack;

	//��ַ��������
	socklen_t slen = sizeof(m_ThreadTask->ConnectSocketAddr_in);

	m_ThreadTask->ConnectSocket = accept(UESlistenSocket, (struct sockaddr *)&m_ThreadTask->ConnectSocketAddr_in, &slen);

	printf("Ip:%s   Client Port:%d   Time:%d   Connect\n", inet_ntoa(m_ThreadTask->ConnectSocketAddr_in.sin_addr), ntohs(m_ThreadTask->ConnectSocketAddr_in.sin_port), time(0), USevent);

	if (m_ThreadTask->ConnectSocket == SOCKET_ERROR)
	{
		return;
	}
	//setsockopt(m_ThreadTask->ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&(m_ThreadTask->Timeout), sizeof(int));

	//�����߳�
	PTP_WORK m_Work = CreateThreadpoolWork(ServerThreadPoolWorking, m_ThreadTask, NULL);
	SubmitThreadpoolWork(m_Work);
}

void Bosncc::On_Read_CallBack(bufferevent * Ubev, void * UVarg)
{
	if (m_Instance == NULL) {
		return;
	}
 
	ThreadTask *m_ThreadTask = (ThreadTask *)UVarg;
	evutil_socket_t fd = bufferevent_getfd(Ubev);

	if (m_ThreadTask->GetTaskMode() != 300) {
		return;
	}

	//��ȡ����ͷ
	if (!m_ThreadTask->DataLenth_All_Done) {
		int ReadCount = bufferevent_read(Ubev, &(m_ThreadTask->DataLenth_All), sizeof(unsigned int));
		if (ReadCount > 0) {
			m_ThreadTask->DataLenth_All_Done = true;
			m_ThreadTask->DataOperationStatus == false;
		}
	}
	if (!m_ThreadTask->EncryptDone && m_ThreadTask->DataLenth_All_Done) {
		int ReadCount = bufferevent_read(Ubev, &(m_ThreadTask->Encrypt), sizeof(bool));
		if (ReadCount > 0) {
			m_ThreadTask->EncryptDone = true;
		}
	}
	if (!m_ThreadTask->Verification_Done && m_ThreadTask->EncryptDone) {
		int ReadCount = bufferevent_read(Ubev, &(m_ThreadTask->Verification), sizeof(unsigned int));
		if (ReadCount > 0) {
			m_ThreadTask->Verification_Done = true;
		}
	}

	//����ͷЧ��
	if (m_ThreadTask->Verification_Ok == false && m_ThreadTask->Verification_Done) {
		if (m_ThreadTask->DataOperationReady() < 0) {
			closesocket(fd);
			event_base_loopbreak(m_ThreadTask->Threadbase);//��ֹ�¼�ѭ��
			return;
		}
	}

	//��ȡ����
	if (m_ThreadTask->Verification_Ok && m_ThreadTask->DataLenth_Remaining) {
		int Readcount = bufferevent_read(Ubev, (m_ThreadTask->DataBuff + m_ThreadTask->DataLenth_Already), m_ThreadTask->DataLenth_Remaining);
		m_ThreadTask->DataLenth_Already += Readcount;
		m_ThreadTask->DataLenth_Remaining -= Readcount;
	}

	//���ݶ�ȡ���
	if (m_ThreadTask->Verification_Ok && m_ThreadTask->DataLenth_Remaining == 0) {
		if (m_ThreadTask->DataOperationEnd() == 0) {
			//�ص�ҵ����
			switch (m_ThreadTask->Business_CallBack(&(m_ThreadTask->m_ConnectProtocol), 300))
			{
			case 1:
				m_ThreadTask->SetTaskMode(100);//���÷���Ϊ����
				bufferevent_enable(Ubev, EV_WRITE | EV_PERSIST);
				break;
			case 2:
				closesocket(fd);
				event_base_loopbreak(m_ThreadTask->Threadbase);//��ֹ�¼�ѭ��
			default:
				closesocket(fd);
				event_base_loopbreak(m_ThreadTask->Threadbase);//��ֹ�¼�ѭ��
				break;
			}
		}
		else {
			closesocket(fd);
			event_base_loopbreak(m_ThreadTask->Threadbase);//��ֹ�¼�ѭ��
		}
	}
}

void Bosncc::On_Error_CallBack(bufferevent * Ubev, short USevent, void * UVarg)
{
	if (m_Instance == NULL) {
		return;
	}
	/*
	USevent:

	#define BEV_EVENT_READING	1	< error encountered while reading
	#define BEV_EVENT_WRITING	2	< error encountered while writing
	#define BEV_EVENT_EOF		16	< eof file reached
	#define BEV_EVENT_ERROR		32	< unrecoverable error encountered
	#define BEV_EVENT_TIMEOUT	64	< user-specified timeout reached
	#define BEV_EVENT_CONNECTED	128	< connect operation finished.
	*/
	evutil_socket_t fd = bufferevent_getfd(Ubev);
	closesocket(fd);
	ThreadTask *m_ThreadTask = (ThreadTask *)UVarg;
	printf("Ip:%s   Client Port:%d   Time:%d   Disconnect<%d>\n", inet_ntoa(m_ThreadTask->ConnectSocketAddr_in.sin_addr), ntohs(m_ThreadTask->ConnectSocketAddr_in.sin_port), time(0), USevent);
}

void Bosncc::On_Write_CallBack(bufferevent * Ubev, void * UVarg)
{
	if (m_Instance == NULL) {
		return;
	}

	ThreadTask *m_ThreadTask = (ThreadTask *)UVarg;
	evutil_socket_t fd = bufferevent_getfd(Ubev);

	if (m_ThreadTask->GetTaskMode() != 100) {
		return;
	}
	//׼������
	if (m_ThreadTask->DataOperationStatus == false) {
		m_ThreadTask->DataOperationReady();
	}

	//��������ͷ
	if (!m_ThreadTask->DataLenth_All_Done) {
		int SendCount = send(fd, (char *)&(m_ThreadTask->DataLenth_All), sizeof(unsigned int), 0);
		if (SendCount > 0) {
			m_ThreadTask->DataLenth_All_Done = true;
		}
	}
	if (!m_ThreadTask->EncryptDone) {
		int SendCount = send(fd, (char *)&(m_ThreadTask->Encrypt), sizeof(bool), 0);
		if (SendCount > 0) {
			m_ThreadTask->EncryptDone = true;
		}
	}
	if (!m_ThreadTask->Verification_Done) {
		int SendCount = send(fd, (char *)&(m_ThreadTask->Verification), sizeof(unsigned int), 0);
		if (SendCount > 0) {
			m_ThreadTask->Verification_Done = true;
		}
	}

	//��������
	if (m_ThreadTask->Verification_Ok == true && m_ThreadTask->DataLenth_Remaining) {
		int sendcount = send(fd, (char *)(m_ThreadTask->DataBuff + m_ThreadTask->DataLenth_Already), m_ThreadTask->DataLenth_Remaining, 0);
		m_ThreadTask->DataLenth_Already += sendcount;
		m_ThreadTask->DataLenth_Remaining -= sendcount;
	}

	//���ݷ������
	if (m_ThreadTask->Verification_Ok == true && m_ThreadTask->DataLenth_Remaining == 0) {
		m_ThreadTask->Business_CallBack(&(m_ThreadTask->m_ConnectProtocol), 100);//�ص�����
		if (m_ThreadTask->DataOperationEnd() == 0) {
			m_ThreadTask->SetTaskMode(300);//���÷���Ϊ����
			bufferevent_enable(Ubev, EV_READ | EV_PERSIST);
		}
	}
}

VOID Bosncc::ServerThreadPoolWorking(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	ThreadTask *m_ThreadTask = (ThreadTask *)Context;

	m_ThreadTask->ThreadRegistered();//���߳�ע��

	//ע��һ��bufferevent_socket_new�¼�
	struct bufferevent *bev = bufferevent_socket_new(m_ThreadTask->Threadbase, m_ThreadTask->ConnectSocket, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, On_Read_CallBack, On_Write_CallBack, On_Error_CallBack, m_ThreadTask);// ���ص���д�ص�������ص�

	//���ó�ʱ
	unsigned int m_timeout = m_ThreadTask->GetTimeout();
	struct timeval ReadTimeOut = { m_timeout, 0 };
	struct timeval WriteTimeOut = { m_timeout, 0 };
	bufferevent_set_timeouts(bev, &ReadTimeOut, &WriteTimeOut);

	switch (m_ThreadTask->GetTaskMode() / 100)
	{
	case 1:
		bufferevent_enable(bev, EV_WRITE | EV_PERSIST);
		break;
	case 3:
		bufferevent_enable(bev, EV_READ | EV_PERSIST);
		break;
	default:
		//�ͷ���Դ
		bufferevent_free(bev);
		if (RunModel == BOSNCC_MODEL_SERVER) {
			delete m_ThreadTask;//�����߳�On_Accept_CallBack��������ģ�Ȼ�󽻸����̣߳�����������Ҫ�����ͷ�;
		}
		return VOID();
	}

	event_base_dispatch(m_ThreadTask->Threadbase);

	//�ͷ���Դ
	bufferevent_free(bev);
	delete m_ThreadTask;//�����߳�On_Accept_CallBack��������ģ�Ȼ�󽻸����̣߳�����������Ҫ�����ͷ�;
	return VOID();
}
