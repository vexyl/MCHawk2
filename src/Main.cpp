#include "../include/Server.hpp"
#include "../include/Utils/Clock.hpp"

static Server* server = nullptr;

#ifdef _WIN32
BOOL WINAPI consoleHandler(DWORD signal)
{
	if (server != nullptr)
		server->Shutdown();
	return TRUE;
}
#endif

int main()
{
#ifdef _WIN32
	SetConsoleCtrlHandler(consoleHandler, TRUE);
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

	std::cerr << "Good bye." << std::endl;
	return 0;
}