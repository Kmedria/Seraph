#pragma once
#include <iostream>

enum LogLevel {
	logLevelNone,		// There is no logging enabled.
	logLevelCritical,	// This level is reserved for error messages regarding running of the program in its most basic state.
	logLevelError,		// This level is reserved for error messages ragarding running extra features that are not vital to the program.
	logLevelWarning,	// This level is reserved for messages that do not directly cause errors but may result in undefined behaviour.
	logLevelDebug,		// This level is reserved for messages to track the state of the program.
	logLevelInfo,		// This level provides info on the program as it runs. This info is supposed to be verbose and should only be used if the other logLevels do not suffice.
	logLevelAll			// Everything is printed to the console.
};

// static const char* logLevelNames[logLevelAll + 1] = {"None", "Critical", "Error", "Warning", "Debug", "Info", "All"};

// The padded Names are not hardcoded but rather generated from another program. 
// They are pasted here below as it is not worth storing it in a textfile.

static const char* logLevelPaddedNames[logLevelAll + 1] = { 
	"[  None  ]",
	"[Critical]",
	"[ Error  ]",
	"[Warning ]",
	"[ Debug  ]",
	"[  Info  ]",
	"[  All   ]" };

static LogLevel loggerLevel = logLevelInfo;
static LogLevel lastMessageLogLevel = logLevelInfo;
static const char* loggerName = "[Logger]";

void setLogLevel(LogLevel level);
void setLastMessageLogLevel(LogLevel level);
static void setLogOutputColour(int textColour) { std::cout << "\033[" << textColour << "m"; }

static void setOutColor(LogLevel somelevel) {
	switch (somelevel)
	{
	case logLevelCritical:
		setLogOutputColour(31);
		break;
	case logLevelError:
		setLogOutputColour(91);
		break;
	case logLevelWarning:
		setLogOutputColour(33);
		break;
	case logLevelDebug:
		setLogOutputColour(35);
		break;
	default:
		setLogOutputColour(37);
		break;
	}
	setLastMessageLogLevel(somelevel);
}

LogLevel getLogLevel();
const char* getLoggerName();

template<typename T>
void log(T message, const char* funcSource) { std::cout << getLoggerName() << "[" << funcSource << "]" << message; }

template<typename T>
void log(T message, LogLevel level, const char* funcSource) {

	if (level <= getLogLevel()) {
		setOutColor(level);
		std::cout << getLoggerName() << "[" << funcSource << "]" << logLevelPaddedNames[level] << message << std::endl;
	}
}

template<typename T>
void throwErrorFunc(T message, LogLevel level, const char* funcSource) {

	if (level <= getLogLevel()) {
		setOutColor(level);
		std::cout << std::endl << "Error" << std::endl << getLoggerName() << "[" << funcSource << "]" << logLevelPaddedNames[level] << message << std::endl;
		throw std::runtime_error(message);
	}
};

#ifndef DEBUG
#define logRecord() 
#endif

// Macro code below taken from https://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros

#define LOG_2(message, messageloglevel) {const char* funcSig = __FUNCSIG__;\
log(message, messageloglevel, funcSig);}

#define LOG_1(message) LOG_2(message, logLevelInfo)

#define LOG_0() 

#define FUNC_CHOOSER_LOGGER(_f1, _f2, _f3, ...) _f3
#define FUNC_RECOMPOSER_LOGGER(argsWithParentheses) FUNC_CHOOSER_LOGGER argsWithParentheses
#define CHOOSE_FROM_ARG_COUNT_LOGGER(...) FUNC_RECOMPOSER_LOGGER((__VA_ARGS__, LOG_2, LOG_1, ))
#define NO_ARG_EXPANDER_LOGGER() ,,LOG_0
#define MACRO_CHOOSER_LOGGER(...) CHOOSE_FROM_ARG_COUNT_LOGGER(NO_ARG_EXPANDER_LOGGER __VA_ARGS__ ())
#define logRecord(...) MACRO_CHOOSER_LOGGER(__VA_ARGS__)(__VA_ARGS__)

#define THROW_ERROR_2(message, messageloglevel) {const char* funcSig = __FUNCSIG__;\
throwErrorFunc(message, messageloglevel, funcSig);}

#define THROW_ERROR_1(message) THROW_ERROR__2(message, logLevelError)

#define THROW_ERROR_0() 

#define FUNC_CHOOSER_THROW_ERROR(_f1, _f2, _f3, ...) _f3
#define FUNC_RECOMPOSER_THROW_ERROR(argsWithParentheses) FUNC_CHOOSER_THROW_ERROR argsWithParentheses
#define CHOOSE_FROM_ARG_COUNT_THROW_ERROR(...) FUNC_RECOMPOSER_THROW_ERROR((__VA_ARGS__, THROW_ERROR_2, THROW_ERROR_1,))
#define NO_ARG_EXPANDER_THROW_ERROR() ,,THROW_ERROR_0
#define MACRO_CHOOSER_THROW_ERROR(...) CHOOSE_FROM_ARG_COUNT_THROW_ERROR(NO_ARG_EXPANDER_THROW_ERROR __VA_ARGS__ ())
#define throwError(...) MACRO_CHOOSER_THROW_ERROR(__VA_ARGS__)(__VA_ARGS__)