#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

#include "SocketBuffer.hpp"

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
class TCPSocket final {
public:
	TCPSocket() : m_socketBuffer(kMaxBufferSize), m_port(0) {};
	TCPSocket(SocketType socket) : TCPSocket() { m_socket = socket; }

	~TCPSocket();

	TCPSocket(const TCPSocket&) = default;
	TCPSocket(TCPSocket&&) = default;
	TCPSocket& operator=(const TCPSocket&) = default;
	TCPSocket& operator=(TCPSocket&&) = default;
	
	bool IsActive() { return m_active; }
	uint8_t PeekFirstByte() { return m_socketBuffer.PeekFirstByte(); }
	uint8_t* GetBufferPtr() { return m_socketBuffer.GetBufferPtr(); }
	size_t GetAvailableDataCount() { return m_socketBuffer.GetAvailableDataCount(); }
	
	static void Initialize();
	static void Cleanup();

	virtual void SetIPAddress(std::string address) { m_IPAddress = address; }

	virtual std::string GetIPAddress() const { return m_IPAddress; }
	uint16_t GetPort() const { return m_port;  }

	virtual void Bind(uint16_t port);
	virtual void Listen();
	virtual std::unique_ptr<TCPSocket> Accept() const;
	virtual size_t Poll();
	virtual bool Receive(Utils::BufferStream& bufferStream);
	virtual int Send(const Utils::BufferStream& bufferStream);

private:
	static constexpr unsigned int kMaxBufferSize = 8192;
	SocketBuffer m_socketBuffer;
	bool m_active = true;
	
	SocketType m_socket = INVALIDSOCKET;
	std::string m_IPAddress;
	uint16_t m_port;
};
} // namespace Net

#endif // TCPSOCKET_H_
