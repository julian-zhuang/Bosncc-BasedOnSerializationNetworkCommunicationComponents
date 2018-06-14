#include "ThreadTask.h"

ThreadTask::ThreadTask()
{
	Threadbase = event_base_new();

	DataStage = 0;
	DataLenth_Count = 0;
	DataLenth_Already_Count = 0;
	DataLenth_Remaining_Count = 0;
	Verification = 0;
	EnableSSLConnect = 0;
	SSLConversation = NULL;
	DataBuff = NULL;
	Business_CallBack = NULL;
	Connectport = 8888;
	ConnectAddress = "127.0.0.1";
	TaskMode = 0;
	Timeout = 0;

	ParentThread = GetCurrentThreadId();
	ChildThreadID = 0;
}

ThreadTask::~ThreadTask()
{
	if (DataBuff != NULL) {
		delete DataBuff;
	}
}

void ThreadTask::operator=(ConnectProtocol UConnectProtocol)
{
}

void ThreadTask::operator=(ThreadTask UThreadTask)
{
}

int ThreadTask::SetConnectPort(unsigned int Uport)
{
	if (GetCurrentThreadId() != ParentThread) { return -1; }
	Connectport = Uport;
	return 0;
}

unsigned int ThreadTask::GetConnectPort()
{
	return Connectport;
}

int ThreadTask::SetConnectAddress(std::string UAddress)
{
	if (GetCurrentThreadId() != ParentThread) { return -1; }
	ConnectAddress = UAddress;
}

std::string ThreadTask::GetConnectAddress()
{
	return ConnectAddress;
}

int ThreadTask::Set_BF_Callback(Business_Function_CallBack UCallback)
{
	Business_CallBack = UCallback;
	return 0;
}

int ThreadTask::Call_BF_Callback(int Arg)
{
	if (Business_CallBack == NULL) {
		return -1;
	}
	return Business_CallBack(m_ParameterGroup, Arg);
}

int ThreadTask::ThreadRegistered()
{
	ChildThreadID = GetCurrentThreadId();
	return 0;
}

int ThreadTask::ThreadUnregister()
{
	if (GetCurrentThreadId() != ChildThreadID) { return -1; }
	ChildThreadID = 0;
	return 0;
}

int ThreadTask::DataPreparation()
{
	if (DataStage == READY_TO_SEND) {
		bufferevent_enable(Bufferevent, EV_WRITE | EV_PERSIST | EV_TIMEOUT);
		return 0;
	}
	if (DataStage == CAN_SEND_DATA_LENTH) {
		DataLenth_Count = m_ConnectProtocol.ByteSize();
		DataLenth_Already_Count = 0;
		return 0;
	}
	if (DataStage == CAN_SEND_DATA_VERIFY) {
		Verification = DataLenth_Count * 2 + 2 + 1024;
		return 0;
	}
	if (DataStage == CAN_SEND_DATA) {
		DataBuff = (char *)malloc(DataLenth_Count);
		memset(DataBuff, 0, DataLenth_Count);
		if (!m_ConnectProtocol.SerializeToArray(DataBuff, DataLenth_Count)) {
			return -1;
		}
		DataLenth_Remaining_Count = DataLenth_Count;
		return 0;
	}
	if (DataStage == CAN_SEND_DATA_END) {
		free(DataBuff);
		DataBuff = NULL;
		DataLenth_Count = 0;
		DataLenth_Already_Count = 0;
		DataLenth_Remaining_Count = 0;
		m_ConnectProtocol.Clear();
	}
	if (DataStage == READY_TO_RECV) {
		m_ConnectProtocol.Clear();
		DataLenth_Count = 0;
		DataLenth_Already_Count = 0;
		DataLenth_Remaining_Count = 0;
		bufferevent_enable(Bufferevent, EV_READ | EV_PERSIST | EV_TIMEOUT);
		return 0;
	}
	if (DataStage == CAN_RECV_DATA_LENTH) {
		return 0;
	}
	if (DataStage == CAN_RECV_DATA_VERIFY) {
		return 0;
	}
	if (DataStage == CAN_RECV_DATA) {
		DataBuff = (char *)malloc(DataLenth_Count);
		memset(DataBuff, 0, DataLenth_Count);
		DataLenth_Already_Count = 0;
		DataLenth_Remaining_Count = DataLenth_Count;
		return 0;
	}
	if (DataStage == CAN_RECV_DATA_END) {
		if (!m_ConnectProtocol.ParseFromArray(DataBuff, DataLenth_Count)) {
			return -1;
		}
		
		free(DataBuff);
		DataBuff = NULL;
		DataLenth_Count = 0;
		DataLenth_Already_Count = 0;
		DataLenth_Remaining_Count = 0;
		m_ParameterGroup.clear();
		Parameter m_Parameter;
		for (int i = 0; i < m_ConnectProtocol.parametergroup_size(); i++) {
			m_Parameter = m_ConnectProtocol.parametergroup(i);
			m_ParameterGroup.insert(ParameterPair(m_Parameter.key(), m_Parameter.value()));
		}
	}
}

int ThreadTask::SetProcessedDone(int Ucode)
{
	return 0;
}

void ThreadTask::SetTimeout(unsigned int UTimeout)
{
	Timeout = UTimeout;
}

unsigned int ThreadTask::GetTimeout()
{
	return Timeout;
}

void ThreadTask::SetTaskMode(unsigned int Umode)
{
	TaskMode = Umode;
}

unsigned int ThreadTask::GetTaskMode()
{
	return TaskMode;
}

int ThreadTask::SetParameterGroup(ParameterGroup & UParameterGroup)
{
	m_ConnectProtocol.clear_parametergroup();
	
	Parameter * m_Parameter = m_ConnectProtocol.add_parametergroup();
	int ParameterCount = 0;
	for (ParameterGroup::iterator Tmp = UParameterGroup.begin(); Tmp != UParameterGroup.end(); Tmp++) {
		m_Parameter = m_ConnectProtocol.add_parametergroup();
		m_Parameter->set_key(Tmp->first);
		m_Parameter->set_value(Tmp->second);
		ParameterCount++;
	}
	return ParameterCount;
}

int ThreadTask::GetDataCount()
{
	return DataLenth_Count;
}

int ThreadTask::SetDataCount(unsigned int USize)
{
	if (GetCurrentThreadId() != ChildThreadID) { return -1; }
	DataLenth_Count = USize;
	return 0;
}

char * ThreadTask::GetDatapointer(unsigned int USize)
{
	if (USize > DataLenth_Count) { return NULL; }
	return DataBuff + USize;
}

int ThreadTask::GetDataAlreadyCount()
{
	return DataLenth_Already_Count;
}

int ThreadTask::SetDataAlreadyCount(int UMod, unsigned int USize)
{
	if (GetCurrentThreadId() != ChildThreadID) { return -1; }
	if (UMod == 1) {
		DataLenth_Already_Count += USize;
		return DataLenth_Already_Count;
	}
	if (UMod == 0) {
		DataLenth_Already_Count -= USize;
		return DataLenth_Already_Count;
	}
	return -2;
}

int ThreadTask::GetDataRemainingCount()
{
	return DataLenth_Remaining_Count;
}

int ThreadTask::SetDataRemainingCount(int UMod, unsigned int USize)
{
	if (GetCurrentThreadId() != ChildThreadID) { return -1; }
	if (UMod == 1) {
		DataLenth_Remaining_Count += USize;
		return DataLenth_Remaining_Count;
	}
	if (UMod == 0) {
		DataLenth_Remaining_Count -= USize;
		return DataLenth_Remaining_Count;
	}
	return -1;
}

int ThreadTask::SetVerificationValue(unsigned int UValue)
{
	Verification = UValue;
	return Verification;
}

int ThreadTask::GetVerification()
{
	if (Verification == (DataLenth_Count * 2 + 2 + 1024)) {
		return 1;
	}
	else {
		return 0;
	}
}

int ThreadTask::GetVerificationValue()
{
	return Verification;
}

int ThreadTask::GetDataStage()
{
	return DataStage;
}

int ThreadTask::SetDataStage(unsigned int UValue)
{
	if (GetCurrentThreadId() == ParentThread) {
		if (ChildThreadID != 0) {
			DataStage = UValue;
			return 0;
		}
	}
	if (GetCurrentThreadId() == ChildThreadID) {
		DataStage = UValue;
		return 0;
	}
	return -1;
}

int ThreadTask::SetEnableSSL(int UMod)
{
	if (GetCurrentThreadId() != ParentThread) { return -1; }
	EnableSSLConnect = UMod;
	return 0;
}

int ThreadTask::GetSSLEnable()
{
	return 	EnableSSLConnect;
}

//void * ThreadTask::GetSSLConversation()
//{
//	//if (GetCurrentThreadId() != ChildThreadID) { return NULL; }
//	return SSLConversation;
//}
