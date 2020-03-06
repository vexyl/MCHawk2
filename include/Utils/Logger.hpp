#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <cstdarg>

#include <iostream>
#include <fstream>
#include <string>

namespace Utils {

class Logger {
public:
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
