#include "../../include/Utils/Logger.hpp"

#include <chrono>
#include <ctime>

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <ctime>

namespace Utils {
Logger::Logger(std::string logFilename) : m_logFileName(logFilename)
{
	m_logFile.open(m_logFileName, std::ios::out | std::ios::app);
}

const std::string CurrentDateTime()
{
	auto clock = std::chrono::system_clock::now();
	std::time_t time = std::chrono::system_clock::to_time_t(clock);

	char buffer[80];
	std::strftime(buffer, 80, "%F %T", std::localtime(&time));

	return std::string(buffer);
}

void Logger::Log(LogLevel logLevel, const char* format, ...)
{
	char buffer[1024];

	std::va_list args;
	va_start(args, format);

	std::vsnprintf(buffer, 1024, format, args);

	va_end(args);

	bool mute = false;
	if (m_verbosityLevel < VerbosityLevel::kNormal) {
		if (logLevel == LogLevel::kDebug || logLevel == LogLevel::kWarning)
			mute = true;
	}

	std::string logFileMessage = "[" + Utils::CurrentDateTime() + "] ";

#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD wOldColorAttrs;
	if (!mute) {
		CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
		GetConsoleScreenBufferInfo(hConsole, &csbiInfo);
		wOldColorAttrs = csbiInfo.wAttributes;
	}
#endif

	if (!mute) {
#ifdef _WIN32
		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
#endif
		std::cerr << "[" + Utils::CurrentDateTime() + "] ";
	}

	if (logLevel == LogLevel::kNormal) {
		if (!mute) {
#ifdef _WIN32
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#endif
		}
	} else if (logLevel == LogLevel::kDebug) {
		if (!mute) {
#ifdef __linux__
			std::cerr << "\033[1;32mDEBUG\033[0m ";
#elif _WIN32
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
			std::cerr << "DEBUG ";
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#else
			std::cerr << "DEBUG ";
#endif
	}

		logFileMessage += "DEBUG ";
} else if (logLevel == LogLevel::kInfo) {
		if (!mute) {
#ifdef __linux__
			std::cerr << "\033[0;32mINFO\033[0m ";
#elif _WIN32
			SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
			std::cerr << "INFO ";
			SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#else
			std::cerr << "INFO ";
#endif
		}

		logFileMessage += "INFO ";
	} else if (logLevel == LogLevel::kWarning) {
		if (!mute) {
#ifdef __linux__
			std::cerr << "\033[0;31mWARNING\033[0m ";
#elif _WIN32
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
			std::cerr << "WARNING ";
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
#else
			std::cerr << "WARNING ";
#endif
		}

		logFileMessage += "WARNING ";
	} else if (logLevel == LogLevel::kError) {
		if (!mute) {
#ifdef __linux__
			std::cerr << "\033[1;31mERROR\033[0m ";
#elif _WIN32
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			std::cerr << "ERROR ";
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
			std::cerr << "ERROR ";
#endif
		}

		logFileMessage += "ERROR ";
	}

	if (!mute) {
		std::cerr << buffer << std::endl;
#ifdef _WIN32
		SetConsoleTextAttribute(hConsole, wOldColorAttrs);
#endif
	}

	logFileMessage += buffer;

	m_logFile << logFileMessage << "\n";
	m_logFile.flush();
}
} // namespace Utils
