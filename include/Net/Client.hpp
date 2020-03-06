#ifndef CLIENT_H_
#define CLIENT_H_

#include "../../include/Net/Socket.hpp"

#include <vector>
#include <memory>

namespace Net {
class Packet;

class Client final {
public:
	Client(Net::Socket* socket) : m_socket(socket) { assert(pid != 255); m_pid = pid++; }

	~Client() { delete m_socket; }

	Client(const Client&) = delete;
	Client& operator=(const Client&) = delete;

	Net::Socket* GetSocket() { return m_socket; }
	std::string GetIPAddress() const { return m_socket->GetIPAddress(); }
	bool KeepAlive() const { return m_keepAlive; }
	bool IsAuthorized() const { return m_isAuthorized; }
	int8_t GetPID() const { return m_pid; }

	void SetKeepAlive(bool keepAlive) { m_keepAlive = keepAlive; }
	void SetAuthorized(bool isAuthorized) { m_isAuthorized = isAuthorized; }

	void Kill() { m_keepAlive = false; }

	void QueuePacket(std::shared_ptr<Net::Packet> packet);
	void ProcessPacketsInQueue();

private:
	static int8_t pid;

	Net::Socket* m_socket = nullptr;
	std::vector<std::shared_ptr<Net::Packet>> m_packetQueue;
	bool m_keepAlive = true, m_isAuthorized = false;

	int8_t m_pid = 0;
};
} // namespace Net

#endif // CLIENT_H_
