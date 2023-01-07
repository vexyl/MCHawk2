#ifndef CLIENT_H_
#define CLIENT_H_

#include "../Net/Socket.hpp"

#include <list>
#include <memory>

#include <cassert>

namespace Net {
class Packet;
class Utils::BufferStream;

class Client final {
public:
	Client(std::unique_ptr<Net::Socket>& socket) : m_socket(std::move(socket)) { assert(sid != 255); m_sid = sid++; }

	std::string GetIPAddress() const { return m_socket->GetIPAddress(); }
	bool KeepAlive() const { return m_keepAlive; }
	bool IsAuthorized() const { return m_isAuthorized; }
	bool IsSocketActive() const { return m_socket->IsActive(); }
	int8_t GetSID() const { return m_sid; }
	uint8_t GetCurrentOpcode() const { return m_socket->PeekFirstByte(); }

	void SetAuthorized(bool isAuthorized) { m_isAuthorized = isAuthorized; }

	void Kill() { m_keepAlive = false; }

	bool TrySocketReceive(size_t packetSize, Utils::BufferStream& reader) const;
	size_t PollSocket() { return m_socket->Poll(); }
	void QueuePacket(std::shared_ptr<Net::Packet> packet);
	void ProcessPacketsInQueue(bool forcePrimaryQueue = false);

private:
	static int8_t sid;

	std::unique_ptr<Net::Socket> m_socket;
	std::list<std::shared_ptr<Net::Packet>> m_packetQueue;
	bool m_keepAlive = true, m_isAuthorized = false;
	int8_t m_sid = 0;
};
} // namespace Net

#endif // CLIENT_H_
