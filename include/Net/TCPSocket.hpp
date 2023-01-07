#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

#include "Socket.hpp"

#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef _WIN32
typedef SOCKET SocketType;
#define INVALIDSOCKET INVALID_SOCKET
#define SOCKETERROR SOCKET_ERROR
#else
typedef int SocketType;
#define INVALIDSOCKET -1
#define SOCKETERROR -1
#endif

namespace Net {
class TCPSocket final : public Socket {
public:
	TCPSocket() : m_port(0) {};
	TCPSocket(SocketType socket) : TCPSocket() { m_socket = socket; }

	~TCPSocket();

	TCPSocket(const TCPSocket&) = default;
	TCPSocket(TCPSocket&&) = default;
	TCPSocket& operator=(const TCPSocket&) = default;
	TCPSocket& operator=(TCPSocket&&) = default;

	static void Initialize();
	static void Cleanup();

	virtual void SetIPAddress(std::string address) override { m_IPAddress = address; }

	virtual std::string GetIPAddress() const override { return m_IPAddress; }
	uint16_t GetPort() const { return m_port;  }

	virtual void Bind(uint16_t port) override;
	virtual void Listen();
	virtual std::unique_ptr<Socket> Accept() const override;
	virtual size_t Poll() override;
	virtual bool Receive(Utils::BufferStream& bufferStream) override;
	virtual int Send(const Utils::BufferStream& bufferStream) override;

private:
	SocketType m_socket = INVALIDSOCKET;
	std::string m_IPAddress;
	uint16_t m_port;
};
} // namespace Net

#endif // TCPSOCKET_H_
