#ifndef EVENT_H_
#define EVENT_H_

#include <functional>
#include <vector>

template <typename T, typename T2>
class Event final {
public:
	typedef std::function<void(T object, const T2&)> EventCallback;

	void Register(EventCallback func)
	{
		m_callbacks.emplace_back(func);
	}

	bool Trigger(T object, const T2& data) const
	{
		if (m_callbacks.empty())
			return false;

		for (const auto& callback : m_callbacks)
			callback(object, data);

		return true;
	}

private:
	std::vector<EventCallback> m_callbacks;
};

#endif // EVENT_H_
