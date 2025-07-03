#include "Logger.h"

void setLogLevel(LogLevel level) {
	loggerLevel = level;
};

void setLastMessageLogLevel(LogLevel level) {
	lastMessageLogLevel = level;
};

LogLevel getLogLevel() {
	return LogLevel(loggerLevel);
}

const char* getLoggerName() {
	return loggerName;
}
