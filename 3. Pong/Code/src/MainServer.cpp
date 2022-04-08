#include "ServerPongApp.h"

#define MY_IMPL 1

int main()
{
	ServerPongApp* server_app = new ServerPongApp();

	if (!server_app->Initialize())
	{
		std::cout << "Server app not initialized correctly." << std::endl;
		return -1;
	}

	while (server_app->IsRunning())
	{
		server_app->Update(0.0f);
	}

	server_app->Terminate();
	delete server_app;

	return 0;
}