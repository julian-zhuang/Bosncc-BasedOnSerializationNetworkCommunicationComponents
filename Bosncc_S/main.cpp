#include <stdio.h>
#include <Bosncc.h>

int CallbackB(ConnectProtocol * UConnectProtocol, int Arg) {
	if (Arg == 100) {
		return 1;
	}
	std::cout << UConnectProtocol->apiname() << std::endl;
	UConnectProtocol->set_apiname("Hello Client");
	return 1;
}

int main(int argc ,char* argv[]) {
	Bosncc * A = Bosncc::GetInstance();
	A->SetRunoModel(BOSNCC_MODEL_SERVER);
	A->SetBusinessFunction(CallbackB);

	A->SetListenport(7777);
	A->SetTimeout(1000);
	int b = A->Init();
	if (b != 0) {
		printf("ini() Error!");
		getchar();
		return -1;
	}
	A->Start();
}