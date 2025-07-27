#include "Server.h"

#include <iostream>
#include <string>

#include "PacketManager.h"

using namespace sniperholdem;

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(605);
	{
		server::PacketManager::Init(100);
		server::PacketManager::Run();

		server::Server gameServer;
		gameServer.InitSocket();
		gameServer.BindAndListen(8888);
		gameServer.StartServer(100);
		

		std::string s;
		std::getline(std::cin, s);

		server::PacketManager::End();

		gameServer.DestroyThread();
		gameServer.~Server();
	}

	int a = 0;
	

}