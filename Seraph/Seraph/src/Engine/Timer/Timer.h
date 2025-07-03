#pragma once
#include <chrono>

#include <string>

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

// Based on the timer from -The Cherno-
class Timer {
public:
	Timer() {
		m_startTimePoint = std::chrono::high_resolution_clock::now();
		m_timerFuncSig = "";
		m_timerName = "";
	}

	Timer(const char* someFuncSig) {
		m_startTimePoint = std::chrono::high_resolution_clock::now();
		m_timerFuncSig = someFuncSig;
		m_timerName = "";
	}

	Timer(const char* someFuncSig, const char* someName) {
		m_startTimePoint = std::chrono::high_resolution_clock::now();
		m_timerFuncSig = someFuncSig;
		m_timerName = someName;
	}

	~Timer() {
		stop();
	}

	void stop() {
		auto endTimePoint = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimePoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

		auto duration = end - start;
		if (m_timerName != "") {
			log("Timer: ["+std::string(m_timerName)+"] has taken " + std::string(std::to_string(duration)) + "us to complete the task.", logLevelInfo, m_timerFuncSig);
		} else {
			log("Timer has taken " + std::string(std::to_string(duration / 1000)) + "ms to complete the task.", logLevelInfo, m_timerFuncSig);
		}
		
	}

private:
	
	std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimePoint;
	const char* m_timerFuncSig;
	const char* m_timerName;

};

#define TIMER_2(someProgramName, timerName) const char* funcSig = __FUNCSIG__;\
Timer someProgramName = Timer(funcSig, timerName);

#define TIMER_1(someProgramName) const char* funcSig = __FUNCSIG__;\
Timer someProgramName = Timer(funcSig);

#define Timer_0() 

#define FUNC_CHOOSER_TIMER(_f1, _f2, _f3, ...) _f3
#define FUNC_RECOMPOSER_TIMER(argsWithParentheses) FUNC_CHOOSER_TIMER argsWithParentheses
#define CHOOSE_FROM_ARG_COUNT_TIMER(...) FUNC_RECOMPOSER_TIMER((__VA_ARGS__, TIMER_2, TIMER_1, ))
#define NO_ARG_EXPANDER_TIMER() ,,TIMER_0
#define MACRO_CHOOSER_TIMER(...) CHOOSE_FROM_ARG_COUNT_TIMER(NO_ARG_EXPANDER_TIMER __VA_ARGS__ ())
#define Timer(...) MACRO_CHOOSER_TIMER(__VA_ARGS__)(__VA_ARGS__)