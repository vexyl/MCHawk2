#ifndef PACKETHANDLERDELEGATE_H_
#define PACKETHANDLERDELEGATE_H_

#include "Packet.hpp"

#include <functional>
#include <vector>

namespace Net {
class Client;

template <typename T>
class PacketHandlerEvent final {
public:
	typedef std::function<void(Client* client, const T&)> PacketHandlerEventCallback;

	void Register(PacketHandlerEventCallback func)
	{
		m_callbacks.emplace_back(func);
	}

	bool Trigger(Client* client, const T& packet) const
	{
		if (m_callbacks.empty())
			return false;

		for (const auto& callback : m_callbacks)
			callback(client, packet);

		return true;
	}

private:
	std::vector<PacketHandlerEventCallback> m_callbacks;
};
} // namespace Net

#endif // PACKETHANDLERDELEGATE_H_