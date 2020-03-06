// Design from SFML's sf::Clock

#include "../../include/Utils/Clock.hpp"

namespace Utils {
void Sleep(int64_t ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

Clock::Clock()
{
	Start();
}

void Clock::Start()
{
	m_start = Steady_Clock::now();
}

Time Clock::Restart()
{
	Time elapsedTime = GetElapsedTime();
	m_start = Steady_Clock::now();
	return elapsedTime;
}

Time Clock::GetElapsedTime() const
{
	return Time(Steady_Clock::now() - m_start);
}
} // namespace Utils