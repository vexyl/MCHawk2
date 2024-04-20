#ifndef CLIENT_H_
#define CLIENT_H_

#include "Socket/TCPSocket.h"

#include <list>
#include <memory>

#include <cassert>

namespace Net {
class Packet;
class Utils::BufferStream;

class Client final {
public:
	typedef std::shared_ptr<Client> Ptr;

	Client(std::unique_ptr<Net::TCPSocket>& socket) : m_socket(std::move(socket)) { }

	std::string GetIPAddress() const { return m_socket->GetIPAddress(); }
	bool KeepAlive() const { return m_keepAlive; }
	bool IsAuthorized() const { return m_isAuthorized; }
	bool IsSocketActive() const { return m_socket->IsActive(); }
	int64_t GetId() const { return m_id; }
	uint8_t GetCurrentOpcode() const { return m_socket->PeekFirstByte(); }

	void SetAuthorized(bool isAuthorized) { m_isAuthorized = isAuthorized; }
	void SetId(int64_t id) { m_id = id; }
	void SetTemporaryPacketQueue(bool useTemporaryQueue);
	void FlushTemporaryPacketQueue();
	void Kill() { m_keepAlive = false; }

	bool TrySocketReceive(size_t packetSize, Utils::BufferStream& reader) const;
	size_t PollSocket() { return m_socket->Poll(); }
	void QueuePacket(std::shared_ptr<Net::Packet> packet, bool useTemporaryQueue=false);
	void ProcessPacketsInQueue();

private:
	std::unique_ptr<Net::TCPSocket> m_socket;
	std::list<std::shared_ptr<Net::Packet>> m_packetQueue, m_temporaryPacketQueue;
	bool m_useTemporaryQueue = false;
	bool m_keepAlive = true, m_isAuthorized = false;
	int64_t m_id = 0;
};
} // namespace Net

#endif // CLIENT_H_
