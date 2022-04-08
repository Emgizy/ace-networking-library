#include "ClientChatApp.h"
#include "SFMLClientChatApp.h"

#define MY_IMPL 1

int main()
{
#if MY_IMPL
	ClientChatApp* client_app = new ClientChatApp();

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
#else
	SFMLClientChatApp* client_app = new SFMLClientChatApp();

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
#endif

	return 0;
}