#include <stdio.h>
#include <Bosncc.h>

int CallbackB(ParameterGroup & UParameterGroup, int Arg) {
	if (Arg == CAN_SEND_DATA_END) {
		printf("CAN_SEND_DATA_END\n");
	}
	if (Arg == CAN_RECV_DATA_END) {
		printf("CAN_RECV_DATA_END\n");
	}
	std::cout << "Username:    " << UParameterGroup["Username"] << std::endl;
	std::cout << "Password:    " << UParameterGroup["Password"] << std::endl;
	return CBR_CLOSE;
}

int main(int argc, char* argv[]) {
	Bosncc * A = Bosncc::GetInstance();
	A->SetRunoModel(BOSNCC_MODEL_SERVER);
	A->SetBusinessFunction(CallbackB);

	A->SetListenport(7777);
	A->SetTimeout(100000);
	A->SetEnableSSL(ENABLE_SSL);
	int b = A->Init();
	if (b != 0) {
		printf("ini() Error!");
		getchar();
		return -1;
	}
	A->Start();
}