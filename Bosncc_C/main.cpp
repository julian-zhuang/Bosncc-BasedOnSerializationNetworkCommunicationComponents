#include <stdio.h>

#include <Bosncc.h>

int CallbackA(ConnectProtocol * UConnectProtocol, int Arg) {
	if (Arg == 100) {
		return 1;
	}
	std::cout << UConnectProtocol->apiname() << std::endl;
	Sleep(1000);
	UConnectProtocol->set_apiname("Hello Server");

	return 1;
}

int main(int argc,char * argv[]) {
	Bosncc * m_Boscnn_C = Bosncc::GetInstance();
	m_Boscnn_C->SetRunoModel(BOSNCC_MODEL_CLIENT);
	m_Boscnn_C->Init();
	ParameterGroup a;
	a.insert(ParameterPair("Username", "TestUsername"));
	a.insert(ParameterPair("Password", "TestPassword"));
	
	ThreadTask *b = new ThreadTask;
	b->Connectport = 7777;// atoi(argv[2]);
	b->ConnectAddress = "127.0.0.1";//argv[1];
	b->SetParameterGroup(a);
	b->SetTaskMode(100);
	b->SetTimeout(1000);
	b->m_ConnectProtocol.set_apiname("Hello Server First");
	b->Business_CallBack = CallbackA;
	m_Boscnn_C->AddTask(b);
	getchar();
	return 0;
}