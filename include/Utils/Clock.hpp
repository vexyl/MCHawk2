#ifndef CLOCK_H_
#define CLOCK_H_

// Design inspired by SFML's sf::Clock

#include <chrono>
#include <thread>

#include <cstdint>

namespace Utils {
void Sleep(int64_t ms);

typedef std::chrono::steady_clock Steady_Clock;

class Time final {
public:
	Time(std::chrono::nanoseconds time) : m_time(time) {}

	int64_t AsMilliseconds() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(m_time).count();
	}

	int64_t AsSeconds() const
	{
		return std::chrono::duration_cast<std::chrono::seconds>(m_time).count();
	}

private:
	std::chrono::nanoseconds m_time;
};

class Clock final {
public:
	Clock();

	void Start();
	Time Restart();
	Time GetElapsedTime() const;

private:
	std::chrono::time_point<Steady_Clock> m_start;
};
} // namespace Utils

#endif // CLOCK_H_