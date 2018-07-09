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
	DWORD dwRetval = getaddrinfo(UThreadTask->GetConnectAddress().c_str(),0,0,&result);
	if (dwRetval != 0) {
		TRACE("getaddrinfo() Error\n");
		return -2;
	}
	if (result->ai_family == AF_INET) {
		UThreadTask->SetConnectAddress(inet_ntoa(((struct sockaddr_in *) result->ai_addr)->sin_addr));
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
	UThreadTask->ConnectSocketAddr_in.sin_port = htons(UThreadTask->GetConnectPort());
	inet_addr(UThreadTask->GetConnectAddress().c_str());

	// ������Bind �ͻ����ǽ�������  
	int ret = connect(UThreadTask->ConnectSocket, (SOCKADDR*)&(UThreadTask->ConnectSocketAddr_in), sizeof(SOCKADDR));//��ʼ����  
	if (ret == SOCKET_ERROR)
	{
		TRACE("connect() Error\n");
		closesocket(UThreadTask->ConnectSocket);
		return -5;
	}
	//�����߳�
	PTP_WORK m_Work = CreateThreadpoolWork(ClienThreadPoolWorking, UThreadTask, NULL);
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
		TRACE("RunModel != BOSNCC_MODEL_SERVER\n");
		return -1;
	}

	int iResult;
	wVersionRequested = MAKEWORD(2, 2);
	iResult = WSAStartup(wVersionRequested, &wsaData);
	if (iResult != 0) {
		TRACE("WSAStartup() Error\n");
		return -2;
	}
	
	TRACE("WSAStartup() ok\n");

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		TRACE("RunModel != BOSNCC_MODEL_SERVER\n");
		return -3;
	}

	Bosncc_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (Bosncc_Socket == INVALID_SOCKET) {
		TRACE("socket() Error\n");
		return -4;
	}
	
	TRACE("socket() ok\n");


	Bosncc_Server_addr_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	Bosncc_Server_addr_in.sin_family = AF_INET;
	Bosncc_Server_addr_in.sin_port = htons(Listenport);

	iResult = bind(Bosncc_Socket, (SOCKADDR*)&Bosncc_Server_addr_in, sizeof(SOCKADDR));

	if (iResult == SOCKET_ERROR) {
		closesocket(Bosncc_Socket);
		WSACleanup();
		TRACE("bind() Error\n");
		return -5;
	}
	TRACE("bind() ok\n");


	iResult = listen(Bosncc_Socket, 5);
	if (iResult == SOCKET_ERROR) {
		TRACE("listen() Error\n");
		return -6;
	}
	TRACE("listen() ok\n");

	WindowsThreadPool = CreateThreadpool(NULL);

	if (WindowsThreadPool == NULL) {
		TRACE("CreateThreadpool() Error\n");
		return -7;
	}
	TRACE("CreateThreadpool() ok\n");

	//�¼�ע��
	evutil_make_socket_nonblocking(Bosncc_Socket);//����socket������ģʽ
	evutil_make_listen_socket_reuseable(Bosncc_Socket);//���õ�ַ����

	Main_Base = event_base_new();
	if (Main_Base == NULL) {
		TRACE("event_base_new() Error\n");
		return -8;
	}
	TRACE("event_base_new() ok\n");

	/*
	����socket�Ŀɶ��¼���������¿ͻ��˵���socket�����Ϊ�ɶ�״̬���ڿɶ���ʱ�򴥷��¼�����������Ӧ�Ļص�����
	*/
	struct event *listen_event = event_new(Main_Base, Bosncc_Socket, EV_READ | EV_PERSIST, On_Accept_CallBack, (void*)Main_Base);
	event_add(listen_event, NULL);

	if (EnableSSLConnect == ENABLE_SSL) {
		if (SSLInit()) {
			TRACE("SSLInit() Error\n");
			return -9;
		}
		TRACE("SSLInit() ok\n");
	}
	return 0;
}

int Bosncc::SSLInit()
{
	if (SSLInitComplete) {
		return 0;
	}

	/* �����û�������֤�飬 ��֤���������͸��ͻ��ˡ� ֤��������й�Կ */
	if (SSL_CTX_use_certificate_file(ServerCtx, "cert.crt", SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stdout);
		TRACE("SSL_CTX_use_certificate_file() Error\n");
		return -2;
	}
	TRACE("SSL_CTX_use_certificate_file() Ok\n");

	/* �����û�˽Կ */
	if (SSL_CTX_use_PrivateKey_file(ServerCtx, "rsa_private.key", SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stdout);
		TRACE("SSL_CTX_use_PrivateKey_file() Error\n");
		return -3;
	}
	TRACE("SSL_CTX_use_PrivateKey_file() Ok\n");

	/* ����û�˽Կ�Ƿ���ȷ */
	if (!SSL_CTX_check_private_key(ServerCtx)) {
		ERR_print_errors_fp(stdout);
		TRACE("SSL_CTX_check_private_key() Error\n");
		return -4;
	}
	TRACE("SSL_CTX_check_private_key() Ok\n");
	return 0;
}

int Bosncc::Start()
{
	if (RunModel != BOSNCC_MODEL_SERVER) {
		TRACE("RunModel != BOSNCC_MODEL_SERVER Error\n");
		return -1;
	}
	if (Started) {
		TRACE("Server Running,Can't change\n");
		return -2;
	}
	//��ʼ�¼�ѭ��
	Started = true;
	printf("Server Starting\n");
	event_base_dispatch(Main_Base);
	Started = false;
	printf("Server Stop\n");
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

int Bosncc::SetSSLCertificate(std::string UFilePath)
{
	return 0;
}

int Bosncc::SetSSLCertificate(std::string UPublicKey, std::string UPrivateKey, unsigned int KeyLenth)
{
	return 0;
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

int Bosncc::SetEnableSSL(unsigned int UValue)
{
	if (Started) {
		return -1;
	}
	if (UValue == DISABLE_SSL) {
		EnableSSLConnect = UValue;
		return 0;
	}
	if (UValue == ENABLE_SSL) {
		/* SSL ���ʼ�� */
		SSL_library_init();

		/* �������� SSL �㷨 */
		OpenSSL_add_all_algorithms();

		/* �������� SSL ������Ϣ */
		SSL_load_error_strings();

		SSLInitComplete = true;

		/* �� SSL V2 �� V3 ��׼���ݷ�ʽ����һ�� SSL_CTX ���� SSL Content Text */
		ServerCtx = SSL_CTX_new(SSLv23_server_method());

		/* Ҳ������ SSLv2_server_method() �� SSLv3_server_method() ������ʾ V2 �� V3��׼ */
		if (ServerCtx == NULL) {
			ERR_print_errors_fp(stdout);
			TRACE("SSL_CTX_new() Error\n");
			return -1;
		}
		TRACE("SSL_CTX_new() Ok\n");
		EnableSSLConnect = UValue;
		return 0;
	}
	return -1;
}

int Bosncc::GetEnableSSL()
{
	if (EnableSSLConnect == ENABLE_SSL) {
		return 1;
	}
	else {
		return 0;
	}
}

Bosncc::Bosncc()
{
	Started = false;
	SSLInitComplete = false;	
	CreatInstance = true;	
	EnableSSLConnect = DISABLE_SSL;
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

	m_ThreadTask->SetTimeout(Bosncc::GetInstance()->Timeout);
	
	m_ThreadTask->Set_BF_Callback(Bosncc::GetInstance()->Business_CallBack);

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
	int Readcount = 0;

	if (m_ThreadTask->GetDataStage() == READY_TO_RECV) {
		if (m_ThreadTask->DataPreparation() == 0) {
			m_ThreadTask->SetDataStage(CAN_RECV_DATA_LENTH);
			m_ThreadTask->DataPreparation();
		}
	}

	if (m_ThreadTask->GetDataStage() == CAN_RECV_DATA_LENTH) {
		unsigned int m_DataLenth = 0;
		Readcount = bufferevent_read(Ubev, (char*)&(m_DataLenth), sizeof(unsigned int));
		if (Readcount > 0) {
			TRACE("���ݴ�С(Recv)  %d   %d\n", Readcount, m_DataLenth);
			m_ThreadTask->SetDataCount(m_DataLenth);
			m_ThreadTask->SetDataStage(CAN_RECV_DATA_VERIFY);
			m_ThreadTask->DataPreparation();
		}
	}

	if (m_ThreadTask->GetDataStage() == CAN_RECV_DATA_VERIFY) {
		unsigned int m_Verification;
		Readcount = bufferevent_read(Ubev, (char*)&(m_Verification), sizeof(unsigned int));
		if (Readcount > 0) {
			TRACE("����Ч��(Recv)  %d   %d\n", Readcount, m_Verification);
			m_ThreadTask->SetVerificationValue(m_Verification);
		}
		if (m_ThreadTask->GetVerification()) {
			TRACE("Ч��ɹ�  %d   %d\n", 1, m_Verification);
			bufferevent_setwatermark(Ubev, EV_READ, 0, 0);//��֤����ͷ��Ч��λ�Ķ�ȡ����
			m_ThreadTask->SetDataStage(CAN_RECV_DATA);
			m_ThreadTask->DataPreparation();
		}
		else {
			closesocket(fd);
			event_base_loopbreak(m_ThreadTask->Threadbase);//��ֹ�¼�ѭ��
			TRACE("Ч��ʧ��  %d   %d\n", 0, m_Verification);
			return;
		}
	}

	//��ȡ����
	if (m_ThreadTask->GetDataStage() == CAN_RECV_DATA) {
		Readcount = bufferevent_read(Ubev, (m_ThreadTask->GetDatapointer(m_ThreadTask->GetDataAlreadyCount())), m_ThreadTask->GetDataRemainingCount());
		m_ThreadTask->SetDataAlreadyCount(1, Readcount);
		m_ThreadTask->SetDataRemainingCount(0, Readcount);
		if (m_ThreadTask->GetDataRemainingCount() == 0) {
			m_ThreadTask->SetDataStage(CAN_RECV_DATA_END);
			if (m_ThreadTask->DataPreparation() < 0) {
				printf("�����л�ʧ��!\n");
			}
		}
	}

	//���ݶ�ȡ���
	if (m_ThreadTask->GetDataStage() == CAN_RECV_DATA_END) {
		//�ص�ҵ����
		switch (m_ThreadTask->Call_BF_Callback(300))
		{
		case 1:
			m_ThreadTask->SetDataStage(CAN_SEND_DATA_LENTH);
			m_ThreadTask->DataPreparation();
			bufferevent_enable(Ubev, EV_WRITE | EV_PERSIST | EV_TIMEOUT);
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
}

void Bosncc::On_Read_CallBack_SSL(bufferevent * Ubev, void * UVarg)
{
	if (m_Instance == NULL) {
		return;
	}
 
	ThreadTask *m_ThreadTask = (ThreadTask *)UVarg;
	evutil_socket_t fd = bufferevent_getfd(Ubev);
	SSL* SSL_fd = (SSL*)m_ThreadTask->SSLConversation;
	int Readcount = 0;

	if (m_ThreadTask->GetDataStage() == READY_TO_RECV) {
		if (m_ThreadTask->DataPreparation() == 0) {
			m_ThreadTask->SetDataStage(CAN_RECV_DATA_LENTH);
			m_ThreadTask->DataPreparation();
		}
	}

	if (m_ThreadTask->GetDataStage() == CAN_RECV_DATA_LENTH) {
		unsigned int m_DataLenth = 0;
		Readcount = SSL_read(SSL_fd, (char*)&(m_DataLenth), sizeof(unsigned int));
		if (Readcount > 0) {
			TRACE("���ݴ�С(Recv)  %d   %d\n", Readcount, m_DataLenth);
			m_ThreadTask->SetDataCount(m_DataLenth);
			m_ThreadTask->SetDataStage(CAN_RECV_DATA_VERIFY);
			m_ThreadTask->DataPreparation();
		}
	}

	if (m_ThreadTask->GetDataStage() == CAN_RECV_DATA_VERIFY) {
		unsigned int m_Verification;
		Readcount = SSL_read(SSL_fd, (char*)&(m_Verification), sizeof(unsigned int));
		if (Readcount > 0) {
			TRACE("����Ч��(Recv)  %d   %d\n", Readcount, m_Verification);
			m_ThreadTask->SetVerificationValue(m_Verification);
		}
		if (m_ThreadTask->GetVerification()) {
			TRACE("Ч��ɹ�  %d   %d\n", 1, m_Verification);
			bufferevent_setwatermark(Ubev, EV_READ, 0, 0);//��֤����ͷ��Ч��λ�Ķ�ȡ����
			m_ThreadTask->SetDataStage(CAN_RECV_DATA);
			m_ThreadTask->DataPreparation();
		}
		else {
			closesocket(fd);
			event_base_loopbreak(m_ThreadTask->Threadbase);//��ֹ�¼�ѭ��
			TRACE("Ч��ʧ��  %d   %d\n", 0, m_Verification);
			return;
		}
	}

	//��ȡ����
	if (m_ThreadTask->GetDataStage() == CAN_RECV_DATA) {
		Readcount = SSL_read(SSL_fd, (m_ThreadTask->GetDatapointer(m_ThreadTask->GetDataAlreadyCount())), m_ThreadTask->GetDataRemainingCount());
		m_ThreadTask->SetDataAlreadyCount(1, Readcount);
		m_ThreadTask->SetDataRemainingCount(0, Readcount);
		if (m_ThreadTask->GetDataRemainingCount() == 0) {
			m_ThreadTask->SetDataStage(CAN_RECV_DATA_END);
			if (m_ThreadTask->DataPreparation() < 0) {
				printf("�����л�ʧ��!\n");
			}
		}
	}

	//���ݶ�ȡ���
	if (m_ThreadTask->GetDataStage() == CAN_RECV_DATA_END) {
		//�ص�ҵ����
		switch (m_ThreadTask->Call_BF_Callback(300))
		{
		case 1:
			m_ThreadTask->SetDataStage(CAN_SEND_DATA_LENTH);
			m_ThreadTask->DataPreparation();
			bufferevent_enable(Ubev, EV_WRITE | EV_PERSIST | EV_TIMEOUT);
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
	int SendCount = 0;

	if (m_ThreadTask->GetDataStage() == READY_TO_SEND) {
		if (m_ThreadTask->DataPreparation() == 0) {
			m_ThreadTask->SetDataStage(CAN_SEND_DATA_LENTH);
			m_ThreadTask->DataPreparation();
		}
	}
	//�������ݴ�С
	if (m_ThreadTask->GetDataStage() == CAN_SEND_DATA_LENTH) {
		unsigned int m_DataLenth = m_ThreadTask->GetDataCount();
		SendCount = send(fd, (char*)&(m_DataLenth), sizeof(unsigned int), 0);
		if (SendCount > 0) {
			TRACE("���ݴ�С(Send)  %d   %d ------>ok\n", SendCount, m_DataLenth);
			m_ThreadTask->SetDataStage(CAN_SEND_DATA_VERIFY);
			m_ThreadTask->DataPreparation();
		}
		else {
			TRACE("���ݴ�С(Send)  %d   %d ------>Error\n", SendCount, m_DataLenth);
		}
	}
	//��������Ч��
	if (m_ThreadTask->GetDataStage() == CAN_SEND_DATA_VERIFY) {
		unsigned int m_Verification = m_ThreadTask->GetVerificationValue();
		SendCount = send(fd, (char*)&(m_Verification), sizeof(unsigned int), 0);
		if (SendCount > 0) {
			TRACE("����Ч��(Send)  %d   %d ------>ok\n", SendCount, m_Verification);
			m_ThreadTask->SetDataStage(CAN_SEND_DATA);
			m_ThreadTask->DataPreparation();
		}
	}

	//��������
	if (m_ThreadTask->GetDataStage() == CAN_SEND_DATA) {
		SendCount = send(fd, (char *)(m_ThreadTask->GetDatapointer(m_ThreadTask->GetDataAlreadyCount())), m_ThreadTask->GetDataRemainingCount(), 0);
		TRACE("��������(Send)  %d   %d ------>ok\n", SendCount, m_ThreadTask->GetDataRemainingCount());
		m_ThreadTask->SetDataRemainingCount(0, SendCount);
		m_ThreadTask->SetDataAlreadyCount(1, SendCount);
		if (m_ThreadTask->GetDataRemainingCount() == 0) {
			m_ThreadTask->SetDataStage(CAN_SEND_DATA_END);
			m_ThreadTask->DataPreparation();
		}
	}

	//���ݷ������
	if (m_ThreadTask->GetDataStage() == CAN_SEND_DATA_END) {
		switch (m_ThreadTask->Call_BF_Callback(CAN_SEND_DATA_END))
		{
		case CBR_RECV:
			m_ThreadTask->SetTaskMode(300);//���÷���Ϊ����
			bufferevent_enable(Ubev, EV_READ | EV_PERSIST);
			//���ö�д�ߵ�ˮλ
			bufferevent_setwatermark(Ubev, EV_READ, 4, 4);//��֤����ͷ��Ч��λ�Ķ�ȡ����
			break;
		case CBR_ALWAYS_RECV:
			break;
		case CBR_SEND:
			m_ThreadTask->SetTaskMode(300);//���÷���Ϊ����
			bufferevent_enable(Ubev, EV_WRITE | EV_PERSIST);
			//���ö�д�ߵ�ˮλ
			bufferevent_setwatermark(Ubev, EV_READ, 0, 0);//��֤����ͷ��Ч��λ�Ķ�ȡ����
			break;
		case CBR_CLOSE:
			closesocket(fd);
			break;
		default:
			closesocket(fd);
			break;
		}
	}
}

void Bosncc::On_Write_CallBack_SSL(bufferevent * Ubev, void * UVarg)
{
	if (m_Instance == NULL) {
		return;
	}

	ThreadTask *m_ThreadTask = (ThreadTask *)UVarg;
	evutil_socket_t fd = bufferevent_getfd(Ubev);
	SSL* SSL_fd = (SSL*)m_ThreadTask->SSLConversation;

	int SendCount = 0;

	if (m_ThreadTask->GetDataStage() == READY_TO_SEND) {
		if (m_ThreadTask->DataPreparation() == 0) {
			m_ThreadTask->SetDataStage(CAN_SEND_DATA_LENTH);
			m_ThreadTask->DataPreparation();
		}
	}
	//�������ݴ�С
	if (m_ThreadTask->GetDataStage() == CAN_SEND_DATA_LENTH) {
		unsigned int m_DataLenth = m_ThreadTask->GetDataCount();
		SendCount = SSL_write(SSL_fd, (char*)&(m_DataLenth), sizeof(unsigned int));
		if (SendCount > 0) {
			TRACE("���ݴ�С(Send)  %d   %d ------>ok\n", SendCount, m_DataLenth);
			m_ThreadTask->SetDataStage(CAN_SEND_DATA_VERIFY);
			m_ThreadTask->DataPreparation();
		}
		else {
			TRACE("���ݴ�С(Send)  %d   %d ------>Error\n", SendCount, m_DataLenth);
		}
	}
	//��������Ч��
	if (m_ThreadTask->GetDataStage() == CAN_SEND_DATA_VERIFY) {
		unsigned int m_Verification = m_ThreadTask->GetVerificationValue();
		SendCount = SSL_write(SSL_fd, (char*)&(m_Verification), sizeof(unsigned int));
		if (SendCount > 0) {
			TRACE("����Ч��(Send)  %d   %d ------>ok\n", SendCount, m_Verification);
			m_ThreadTask->SetDataStage(CAN_SEND_DATA);
			m_ThreadTask->DataPreparation();
		}
	}

	//��������
	if (m_ThreadTask->GetDataStage() == CAN_SEND_DATA) {
		SendCount = SSL_write(SSL_fd, (char *)(m_ThreadTask->GetDatapointer(m_ThreadTask->GetDataAlreadyCount())), m_ThreadTask->GetDataRemainingCount());
		TRACE("��������(Send)  %d   %d ------>ok\n", SendCount, m_ThreadTask->GetDataRemainingCount());
		m_ThreadTask->SetDataRemainingCount(0, SendCount);
		m_ThreadTask->SetDataAlreadyCount(1, SendCount);
		if (m_ThreadTask->GetDataRemainingCount() == 0) {
			m_ThreadTask->SetDataStage(CAN_SEND_DATA_END);
			m_ThreadTask->DataPreparation();
		}
	}

	//���ݷ������
	if (m_ThreadTask->GetDataStage() == CAN_SEND_DATA_END) {
		switch (m_ThreadTask->Call_BF_Callback(CAN_SEND_DATA_END))
		{
		case CBR_RECV:
			m_ThreadTask->SetTaskMode(300);//���÷���Ϊ����
			bufferevent_enable(Ubev, EV_READ | EV_PERSIST);
			//���ö�д�ߵ�ˮλ
			bufferevent_setwatermark(Ubev, EV_READ, 4, 4);//��֤����ͷ��Ч��λ�Ķ�ȡ����
			break;
		case CBR_ALWAYS_RECV:
			break;
		case CBR_SEND:
			m_ThreadTask->SetTaskMode(300);//���÷���Ϊ����
			bufferevent_enable(Ubev, EV_WRITE | EV_PERSIST);
			//���ö�д�ߵ�ˮλ
			bufferevent_setwatermark(Ubev, EV_READ, 0, 0);//��֤����ͷ��Ч��λ�Ķ�ȡ����
			break;
		case CBR_CLOSE:
			closesocket(fd);
			break;
		default:
			closesocket(fd);
			break;
		}
	}
}

VOID Bosncc::ServerThreadPoolWorking(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	ThreadTask *m_ThreadTask = (ThreadTask *)Context;
	SSL* m_SSL = (SSL*)m_ThreadTask->SSLConversation;

	m_ThreadTask->ThreadRegistered();//���߳�ע��

	//ע��һ��bufferevent_socket_new�¼�
	m_ThreadTask->Bufferevent = bufferevent_socket_new(m_ThreadTask->Threadbase, m_ThreadTask->ConnectSocket, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(m_ThreadTask->Bufferevent, On_Read_CallBack, On_Write_CallBack, On_Error_CallBack, m_ThreadTask);
	
	//���ó�ʱ
	unsigned int m_timeout = m_ThreadTask->GetTimeout();
	struct timeval ReadTimeOut = { m_timeout, 0 };
	struct timeval WriteTimeOut = { m_timeout, 0 };
	bufferevent_set_timeouts(m_ThreadTask->Bufferevent, &ReadTimeOut, &WriteTimeOut);

	//�жϿͻ����Ƿ��������
	unsigned int SSL_Status;
	unsigned int RecvCount;
	RecvCount = recv(m_ThreadTask->ConnectSocket, (char*)&SSL_Status, sizeof(unsigned int), 0);
	if (RecvCount <= 0) {
		closesocket(m_ThreadTask->ConnectSocket);
		delete m_ThreadTask;
		return VOID();
	}

	if (SSL_Status != DISABLE_SSL && SSL_Status != ENABLE_SSL) {
		bufferevent_free(m_ThreadTask->Bufferevent);
		if (SSL_Status == 200) {
			SSL_shutdown(m_SSL);
			SSL_free(m_SSL);
		}
		closesocket(m_ThreadTask->ConnectSocket);
		printf("Ip:%s   Client Port:%d   Time:%d   Disconnect(SSL_STATUS ERROR)\n", inet_ntoa(m_ThreadTask->ConnectSocketAddr_in.sin_addr), ntohs(m_ThreadTask->ConnectSocketAddr_in.sin_port), time(0));
		delete m_ThreadTask;//�����߳�On_Accept_CallBack��������ģ�Ȼ�󽻸����̣߳�����������Ҫ�����ͷ�;
		return VOID();
	}
	if (SSL_Status == DISABLE_SSL) {
		// ���ص���д�ص�������ص�
		bufferevent_setcb(m_ThreadTask->Bufferevent, On_Read_CallBack, On_Write_CallBack, On_Error_CallBack, m_ThreadTask);
	}
	if (SSL_Status == ENABLE_SSL) {

		m_SSL = SSL_new(Bosncc::GetInstance()->ServerCtx);

		SSL_set_fd(m_SSL, m_ThreadTask->ConnectSocket);

		if (SSL_accept(m_SSL) != 1) {
			printf("SSL_accept Error\n");
			SSL_free(m_SSL);
			closesocket(m_ThreadTask->ConnectSocket);
			printf("Ip:%s   Client Port:%d   Time:%d   Disconnect\n", inet_ntoa(m_ThreadTask->ConnectSocketAddr_in.sin_addr), ntohs(m_ThreadTask->ConnectSocketAddr_in.sin_port), time(0));
			delete m_ThreadTask;
			return VOID();
		}
		// ���ص���д�ص�������ص�
		bufferevent_setcb(m_ThreadTask->Bufferevent, On_Read_CallBack_SSL, On_Write_CallBack_SSL, On_Error_CallBack, m_ThreadTask);
	}
	
	m_ThreadTask->SetDataStage(READY_TO_RECV);
	m_ThreadTask->DataPreparation();
	//��ʼ�¼�ѭ��
	int m_return = event_base_dispatch(m_ThreadTask->Threadbase);

	if (m_return != 0) {
		TRACE("event_base_dispatch Error %d\n", m_return);
	}

	//�ͷ���Դ
	bufferevent_free(m_ThreadTask->Bufferevent);
	if (SSL_Status == 200) {
		SSL_shutdown(m_SSL);
		SSL_free(m_SSL);
	}
	closesocket(m_ThreadTask->ConnectSocket);
	printf("Ip:%s   Client Port:%d   Time:%d   Disconnect\n", inet_ntoa(m_ThreadTask->ConnectSocketAddr_in.sin_addr), ntohs(m_ThreadTask->ConnectSocketAddr_in.sin_port), time(0));
	delete m_ThreadTask;//�����߳�On_Accept_CallBack��������ģ�Ȼ�󽻸����̣߳�����������Ҫ�����ͷ�;
	return VOID();
}

VOID Bosncc::ClienThreadPoolWorking(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	ThreadTask *m_ThreadTask = (ThreadTask *)Context;

	m_ThreadTask->ThreadRegistered();//���߳�ע��
	
	//ע��һ��bufferevent_socket_new�¼�
	m_ThreadTask->Bufferevent = bufferevent_socket_new(m_ThreadTask->Threadbase, m_ThreadTask->ConnectSocket, BEV_OPT_CLOSE_ON_FREE);
	
	//���ó�ʱ
	unsigned int m_timeout = m_ThreadTask->GetTimeout();
	struct timeval ReadTimeOut = { m_timeout, 0 };
	struct timeval WriteTimeOut = { m_timeout, 0 };
	bufferevent_set_timeouts(m_ThreadTask->Bufferevent, &ReadTimeOut, &WriteTimeOut);

	unsigned int m_EnableSSL = Bosncc::GetInstance()->EnableSSLConnect;
	TRACE("%d \n", m_EnableSSL);
	int SendCount = send(m_ThreadTask->ConnectSocket, (char*)&(m_EnableSSL), sizeof(unsigned int), 0);

	if (SendCount <= 0) {
		bufferevent_free(m_ThreadTask->Bufferevent);
		if (RunModel == BOSNCC_MODEL_SERVER) {
			delete m_ThreadTask;//�����߳�On_Accept_CallBack��������ģ�Ȼ�󽻸����̣߳�����������Ҫ�����ͷ�;
		}
		return VOID();
	}

	if (m_EnableSSL == ENABLE_SSL) {
		m_ThreadTask->SSLConversation = SSL_new(Bosncc::GetInstance()->ServerCtx);
		/* �������û��� socket ���뵽 SSL */
		SSL_set_fd((SSL *)m_ThreadTask->SSLConversation, m_ThreadTask->ConnectSocket);
		/* ���� SSL ���� */
		if (SSL_connect((SSL *)m_ThreadTask->SSLConversation) != 0) {
			ERR_print_errors_fp(stdout);
			/* �ͷ� SSL */
			SSL_free((SSL *)m_ThreadTask->SSLConversation);
			//�ͷ���Դ
			std::cout << "SSL_connect()   Error 1s Sleep" << std::endl;
		}
		// ����SSL ���ص���д�ص�������ص�
		bufferevent_setcb(m_ThreadTask->Bufferevent, On_Read_CallBack_SSL, On_Write_CallBack_SSL, On_Error_CallBack, m_ThreadTask);
		
	}
	if (m_EnableSSL == DISABLE_SSL) {
		// ���ö��ص���д�ص�������ص�
		bufferevent_setcb(m_ThreadTask->Bufferevent, On_Read_CallBack, On_Write_CallBack, On_Error_CallBack, m_ThreadTask);
	}

	m_ThreadTask->SetDataStage(READY_TO_SEND);
	if (m_ThreadTask->DataPreparation() != 0) {
		bufferevent_free(m_ThreadTask->Bufferevent);
		if (RunModel == BOSNCC_MODEL_SERVER) {
			delete m_ThreadTask;//�����߳�On_Accept_CallBack��������ģ�Ȼ�󽻸����̣߳�����������Ҫ�����ͷ�;
		}
		return VOID();
	}

	//��ʼ�¼�ѭ��
	int m_return = event_base_dispatch(m_ThreadTask->Threadbase);
	if (m_return != 0) {
		TRACE("event_base_dispatch Error %d\n", m_return);
	}

	//�ͷ���Դ
	bufferevent_free(m_ThreadTask->Bufferevent);
	delete m_ThreadTask;//�����߳�On_Accept_CallBack��������ģ�Ȼ�󽻸����̣߳�����������Ҫ�����ͷ�;
	return VOID();
}
