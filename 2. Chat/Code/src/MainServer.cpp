#include "ServerChatApp.h"
#include "SFMLServerChatApp.h"

#define MY_IMPL 1

int main()
{
#if MY_IMPL
	ServerChatApp* server_app = new ServerChatApp();

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
#else
	SFMLServerChatApp* server_app = new SFMLServerChatApp();

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
#endif

	return 0;
}