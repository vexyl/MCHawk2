#ifndef SocketType_H_
#define SocketType_H_

#include "SocketBuffer.hpp"

namespace Net {
class Socket {
public:
	Socket() : m_socketBuffer(kMaxBufferSize) {}

	virtual ~Socket() {}

	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	virtual void SetIPAddress(std::string address) = 0;

	virtual std::string GetIPAddress() const = 0;

	bool IsActive() { return m_active; }

	uint8_t PeekFirstByte() { return m_socketBuffer.PeekFirstByte(); }
	uint8_t* GetBufferPtr() { return m_socketBuffer.GetBufferPtr(); }
	size_t GetAvailableDataCount() { return m_socketBuffer.GetAvailableDataCount(); }

	virtual void Bind(uint16_t port) = 0;
	virtual void Listen() = 0;
	virtual Socket* Accept() const = 0;
	virtual size_t Poll() = 0;
	virtual bool Receive(Utils::BufferStream& bufferStream) = 0;
	virtual int Send(const Utils::BufferStream& bufferStream) = 0;

protected:
	SocketBuffer m_socketBuffer;
	bool m_active = true;

private:
	static constexpr unsigned int kMaxBufferSize = 8192;
};
} // namespace Net

#endif // SocketType_H_
