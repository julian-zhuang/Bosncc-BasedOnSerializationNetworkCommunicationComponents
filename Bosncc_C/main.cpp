#include <stdio.h>

#include <Bosncc.h>

int CallbackA(ParameterGroup & UParameterGroup, int Arg) {
	if (Arg == CAN_SEND_DATA_END) {
		printf("CAN_SEND_DATA_END\n");
	}
	if (Arg == CAN_RECV_DATA_END) {
		printf("CAN_RECV_DATA_END\n");
	}
	return CBR_CLOSE;
}

int main(int argc, char * argv[]) {
	Bosncc * m_Boscnn_C = Bosncc::GetInstance();
	m_Boscnn_C->SetRunoModel(BOSNCC_MODEL_CLIENT);
	m_Boscnn_C->Init();
	//m_Boscnn_C->SetEnableSSL(ENABLE_SSL);
	ParameterGroup a;
	a.insert(ParameterPair("Username", "TestUsername"));
	a.insert(ParameterPair("Password", "TestPassword"));
	//for (int i = 0; i < 50; i++) {
	ThreadTask *b = new ThreadTask;
	b->SetConnectPort(7777);// = 7777;// atoi(argv[2]);
	b->SetConnectAddress("127.0.0.1");// = "127.0.0.1";//argv[1];
	b->SetParameterGroup(a);
	b->SetTaskMode(100);
	b->SetTimeout(100000);
	b->Set_BF_Callback(CallbackA);
	m_Boscnn_C->AddTask(b);
	//}
	getchar();
	return 0;
}