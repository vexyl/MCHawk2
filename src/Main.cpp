#include "../include/Server.hpp"
#include "../include/Utils/Clock.hpp"

#ifndef _WIN32
#include <signal.h>
#endif

constexpr unsigned int kMaxSleepTime = 33;

Server server;

void Shutdown()
{
	server.Shutdown();
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
	if (sig == SIGINT)
		Shutdown();
}
#endif

int main()
{
#ifdef _WIN32
	SetConsoleCtrlHandler(windowsHandler, TRUE);
#else
	signal(SIGINT, linuxHandler);
	signal(SIGPIPE, linuxHandler);
#endif

	std::cout << "Starting server..." << std::endl;

	server.Init();

	Utils::Clock clock;

	bool running = true;
	while (running) {
		clock.Restart();

		running = server.Update();

		auto ms = clock.GetElapsedTime().AsMilliseconds();
		if (ms < kMaxSleepTime)
			Utils::Sleep(kMaxSleepTime - ms);
	}

	std::cerr << "Goodbye." << std::endl;
	return 0;
}
