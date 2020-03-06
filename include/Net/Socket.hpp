#ifndef SocketType_H_
#define SocketType_H_

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

#include <cstdint>
#include <cassert>

#include "../../include/Utils/BufferStream.hpp"

constexpr auto MAX_PACKET_SIZE = 131 * 2; // AuthenticationPacket = 131 bytes

namespace Net {
// Put directly in Socket class?
class SocketBuffer final {
public:
	SocketBuffer(size_t size) : m_bufferSize(size)
	{
		m_buffer = new uint8_t[m_bufferSize];
	}

	~SocketBuffer()
	{
		delete m_buffer;
	}

	SocketBuffer(const SocketBuffer&) = delete;
	SocketBuffer& operator=(const SocketBuffer&) = delete;

	size_t GetAvailableDataCount() { return m_head; }
	uint8_t* GetBufferPtr() { return m_buffer; }
	size_t GetBufferSize() { return m_bufferSize; }

	uint8_t PeekFirstByte() const
	{
		return m_buffer[0];
	}

	// FIXME: Check boundaries
	void SetHead(size_t head) { m_head = head; }

	// Writes as much data as possible
	size_t Write(void *inData, size_t inDataSize)
	{
		size_t byteCount = std::min(inDataSize, m_bufferSize - m_head);

		std::memcpy(m_buffer + m_head, inData, byteCount);
		m_head += byteCount;

		return byteCount;
	}

	bool Read(void* outData, size_t outDataSize)
	{
		// Not enough available bytes in buffer
		if (outDataSize > m_head)
			return false;

		std::memcpy(outData, m_buffer, outDataSize);

		m_head -= outDataSize;

		// Move what we have left back to beginning of buffer
		std::memmove(m_buffer, m_buffer + outDataSize, m_head);

		return true;
	}

	bool Read(Utils::BufferStream& bufferStream)
	{
		bufferStream.AllocateIfNeeded();
		return Read(bufferStream.GetBufferPtr(), bufferStream.GetBufferSize());
	}

private:
	uint8_t* m_buffer = nullptr;
	size_t m_bufferSize = 0, m_head = 0;
};

class Socket {
public:
	Socket() : m_socketBuffer(MAX_PACKET_SIZE) {}

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
	virtual int Receive(Utils::BufferStream& bufferStream) = 0;
	virtual int Send(const Utils::BufferStream& bufferStream) = 0;

protected:
	SocketBuffer m_socketBuffer;
	bool m_active = true;
};
} // namespace Net

#endif // SocketType_H_
