#include "ClientPongApp.h"


int main()
{

	ClientPongApp* client_app = new ClientPongApp();

	if (!client_app->Initialize())
	{
		std::cout << "Client app not initialized correctly." << std::endl;
		return -1;
	}

	while (client_app->IsRunning())
	{
		client_app->Update(0.0f);
	}

	client_app->Terminate();
	delete client_app;

	return 0;
}