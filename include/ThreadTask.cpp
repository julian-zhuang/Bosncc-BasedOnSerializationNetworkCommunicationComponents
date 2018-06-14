#include "ThreadTask.h"

int ThreadTask::DataOperationEnd()
{
	switch (TaskMode / 100)
	{
	case 1://·¢ËÍ
		DataLenth_All_Done = false;
		DataLenth_Already_Done = false;
		DataLenth_Remaining_Done = false;
		Verification_Done = false;
		Verification_Ok = false;
		delete DataBuff;
		DataBuff = NULL;
		m_ConnectProtocol.Clear();
		DataOperationStatus == true;
		break;
	case 3://½ÓÊÕ
		if (m_ConnectProtocol.ParseFromArray(DataBuff, DataLenth_Already)) {
			DataLenth_All_Done = false;
			DataLenth_Already_Done = false;
			DataLenth_Remaining_Done = false;
			Verification_Done = false;
			Verification_Ok = false;
			delete DataBuff;
			DataBuff = NULL;
			DataOperationStatus == true;
		}
		else {
			return -1;
		}
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

int ThreadTask::DataOperationReady()
{
	switch (TaskMode / 100)
	{
	case 1:
		DataLenth_All = m_ConnectProtocol.ByteSize();
		DataLenth_All_Done = false;

		DataLenth_Already = 0;
		DataLenth_Already_Done = false;

		DataLenth_Remaining = DataLenth_All;
		DataLenth_Remaining_Done = false;

		DataBuff = (char*)malloc(DataLenth_All);
		memset(DataBuff, 0, DataLenth_All);
		m_ConnectProtocol.SerializeToArray(DataBuff, DataLenth_All);

		Verification = DataLenth_All * (Encrypt ? 1 : 2) + (Encrypt ? 1 : 2) + 1024;
		Verification_Ok = true;

		DataOperationStatus == true;
		break;
	case 3:
		if (Verification == DataLenth_All * (Encrypt ? 1 : 2) + (Encrypt ? 1 : 2) + 1024) {
			Verification_Ok = true;
			DataBuff = (char *)malloc(DataLenth_All);
			DataLenth_Remaining = DataLenth_All;
			DataLenth_Already = 0;
			m_ConnectProtocol.Clear();
			return 1;
		}
		else {
			Verification_Ok = false;
			return -1;
		}
		break;
	default:
		break; 
	}
	return 0;
}

ThreadTask::ThreadTask()
{
	Threadbase = event_base_new();
	DataOperationStatus = false;

	DataLenth_All = 0;
	DataLenth_All_Done = false;
	DataLenth_Already = 0;
	DataLenth_Already_Done = false;
	DataLenth_Remaining = 0;
	DataLenth_Remaining_Done = false;

	Verification = 0;
	Verification_Done = false;
	Verification_Ok = false;

	Encrypt = 0;
	EncryptDone = false;

	DataBuff = NULL;

	Connectport = 8888;
	ConnectAddress = "127.0.0.1";
	TaskMode = 0;

	Timeout = 5;

	Business_CallBack = NULL;

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

int ThreadTask::ThreadRegistered()
{
	ChildThreadID = GetCurrentThreadId();
	return 0;
}

int ThreadTask::ThreadUnregister()
{
	return 0;
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
