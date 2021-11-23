#include "../include/Server.hpp"
#include "../include/Utils/Clock.hpp"

#ifndef _WIN32
#include <signal.h>
#endif

static Server* server = nullptr;

void Shutdown()
{
	if (server != nullptr)
		server->Shutdown();
}

#ifdef _WIN32
BOOL WINAPI windowsHandler(DWORD signal)
{
	Shutdown();
	return TRUE;
}
#else
void linuxHandler(int sig)
{
	Shutdown();
}
#endif

int main()
{
#ifdef _WIN32
	SetConsoleCtrlHandler(windowsHandler, TRUE);
#else
	signal(SIGINT, linuxHandler);
#endif

	std::cout << "Starting server..." << std::endl;

	server = Server::GetInstance();
	server->Init();

	Utils::Clock clock;

	bool running = true;
	while (running) {
		clock.Restart();

		running = server->Update();

		auto ms = clock.GetElapsedTime().AsMilliseconds();
		if (ms < 33)
			Utils::Sleep(33 - ms);
	}

	delete server;

	std::cerr << "Goodbye." << std::endl;
	return 0;
}
