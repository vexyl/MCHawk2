#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <cstdarg>

#include <iostream>
#include <fstream>
#include <string>

#pragma region LoggerMacros
#define LOG(...) m_logger->Log(__VA_ARGS__);
#define LOGLEVEL_NORMAL Utils::Logger::LogLevel::kNormal
#define LOGLEVEL_DEBUG Utils::Logger::LogLevel::kDebug
#define LOGLEVEL_INFO Utils::Logger::LogLevel::kInfo
#define LOGLEVEL_WARNING Utils::Logger::LogLevel::kWarning
#define LOGLEVEL_ERROR Utils::Logger::LogLevel::kError
#pragma endregion

namespace Utils {
class Logger {
public:
	typedef std::shared_ptr<Logger> Ptr;
	enum class LogLevel { kNormal, kInfo, kDebug, kWarning, kError };
	enum class VerbosityLevel { kQuiet, kNormal };

	Logger(std::string logFilename);

	void Log(LogLevel logLevel, const char *format, ...);

	void SetVerbosityLevel(VerbosityLevel verbosityLevel) { m_verbosityLevel = verbosityLevel; }

private:
	const std::string m_logFileName;
	std::ofstream m_logFile;
	std::string m_lastDateString;

	VerbosityLevel m_verbosityLevel = VerbosityLevel::kNormal;;
};
} // namespace Utils

#endif // LOGGER_H
