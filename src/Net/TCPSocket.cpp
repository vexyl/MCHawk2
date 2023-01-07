#include "../../include/Net/TCPSocket.hpp"

#include <iostream>

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>
#endif

#ifdef _WIN32
#define errno WSAGetLastError()
#endif

using namespace Net;

TCPSocket::~TCPSocket()
{
#ifdef _WIN32
	closesocket(m_socket);
#else
	close(m_socket);
#endif
}

void TCPSocket::Initialize()
{
#ifdef _WIN32
	WSADATA wsaData;

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cerr << "Failed to initialize TCPSocket: WSAStartup failed" << std::endl;
		std::exit(1);
	}
#endif
}

void TCPSocket::Cleanup()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

void TCPSocket::Bind(uint16_t port)
{
	struct addrinfo* addrInfoResult = nullptr;
	struct addrinfo hints;

	m_port = port;

#ifdef _WIN32
	ZeroMemory(&hints, sizeof(hints));
#else
	memset(&hints, 0, sizeof(hints));
#endif

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int iResult = getaddrinfo(NULL, std::to_string(m_port).c_str(), &hints, &addrInfoResult);
	if (iResult != 0) {
		std::cerr << "Failed to initialize TCPSocket: getaddr info failed" << std::endl;
		std::exit(1);
	}

	m_socket = socket(addrInfoResult->ai_family, addrInfoResult->ai_socktype, addrInfoResult->ai_protocol);
	if (m_socket == INVALIDSOCKET) {
		std::cerr << "Failed to initialize TCPSocket: socket() failed" << std::endl;
		std::exit(1);
	}

#ifdef _WIN32
	// Async (non-blocking) sockets
	u_long mode = 1;
	ioctlsocket(m_socket, FIONBIO, &mode);
#else
	fcntl(m_socket, F_SETFL, fcntl(m_socket, F_GETFL, 0) | O_NONBLOCK);
#endif

	// Disable Nagle's algorithm
	int flag = 1;
	if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) < 0) {
		std::cerr << "Failed to setsockopt() TCP_NODELAY, errno=" << errno << std::endl;
		std::exit(1);
	}

	iResult = bind(m_socket, addrInfoResult->ai_addr, static_cast<int>(addrInfoResult->ai_addrlen));
	if (iResult == SOCKETERROR) {
		std::cerr << "Failed to initialize TCPSocket: bind() failed, errno=" << errno << std::endl;
		std::exit(1);
	}

	freeaddrinfo(addrInfoResult);
}

void TCPSocket::Listen()
{
	if (listen(m_socket, SOMAXCONN) == SOCKETERROR) {
		std::cerr << "Failed to initialize TCPSocket: listen() failed" << std::endl;
		std::exit(1);
	}
}

std::unique_ptr<Socket> TCPSocket::Accept() const
{
	SocketType raw_socket = accept(m_socket, NULL, NULL);

	if (raw_socket == INVALIDSOCKET)
		return nullptr;

#ifdef _WIN32
	u_long mode = 1;
	ioctlsocket(raw_socket, FIONBIO, &mode);
#else
	fcntl(raw_socket, F_SETFL, fcntl(m_socket, F_GETFL, 0) | O_NONBLOCK);
#endif

	sockaddr_in client_info = { 0 };
#ifdef _WIN32
	int addrsize = sizeof(client_info);
#else
	socklen_t addrsize = sizeof(client_info);
#endif
	getpeername(raw_socket, reinterpret_cast<sockaddr*>(&client_info), &addrsize);

	std::string address(inet_ntoa(client_info.sin_addr));

	std::unique_ptr<Socket> socket = std::make_unique<TCPSocket>(raw_socket);
	socket->SetIPAddress(address);

	return std::move(socket);
}

size_t TCPSocket::Poll()
{
	auto buffer = m_socketBuffer.GetBufferPtr();
	auto bufferSize = m_socketBuffer.GetBufferSize();
	auto head = m_socketBuffer.GetAvailableDataCount();

	size_t max_bytes = bufferSize - head;

	int bytesReceived = recv(m_socket, reinterpret_cast<char*>(buffer+head), static_cast<int>(max_bytes), 0);

	if (bytesReceived == SOCKETERROR) {
#ifdef _WIN32
		int result = WSAGetLastError();
		if (result == WSAEWOULDBLOCK) {
			return 0;
		} else if (result == WSAECONNRESET) {
			m_active = false;
			return 0;
		} else {
			std::cerr << "WSAGetLastError: " << result << std::endl;
		}

		return 0;
#else
		return 0;
#endif
	} else if (bytesReceived == 0) {
		m_active = false;
	}

	head += static_cast<size_t>(bytesReceived);
	m_socketBuffer.SetHead(head);

	return head; // Bytes available to read in buffer
}

bool TCPSocket::Receive(Utils::BufferStream& bufferStream)
{
	return m_socketBuffer.Read(bufferStream);
}

int TCPSocket::Send(const Utils::BufferStream& bufferStream)
{
	return send(m_socket, reinterpret_cast<const char*>(bufferStream.GetReadOnlyBufferPtr()), static_cast<int>(bufferStream.GetBufferSize()), 0);
}
