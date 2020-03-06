#ifndef TCPSocketType_H_
#define TCPSocketType_H_

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

	static void Initialize();
	static void Cleanup();

	virtual void SetIPAddress(std::string address) override { m_IPAddress = address; }

	virtual std::string GetIPAddress() const override { return m_IPAddress; }
	uint16_t GetPort() const { return m_port;  }

	virtual void Bind(uint16_t port) override;
	virtual void Listen();
	virtual Socket* Accept() const override;
	virtual size_t Poll() override;
	virtual int Receive(Utils::BufferStream& bufferStream) override;
	virtual int Send(const Utils::BufferStream& bufferStream) override;

private:
	SocketType m_socket = INVALIDSOCKET;
	std::string m_IPAddress;
	uint16_t m_port;
};
} // namespace Net

#endif // TCPSocketType_H_
