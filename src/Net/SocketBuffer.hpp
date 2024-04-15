#ifndef SOCKET_BUFFER_H_
#define SOCKET_BUFFER_H_

#include "../Utils/BufferStream.hpp"

#include <cstdint>

namespace Net {
class SocketBuffer final {
public:
	SocketBuffer(size_t size);

	~SocketBuffer();

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

	size_t Write(void* inData, size_t inDataSize);

	bool Read(void* outData, size_t outDataSize);
	bool Read(Utils::BufferStream& bufferStream);

private:
	uint8_t* m_buffer = nullptr;
	size_t m_bufferSize = 0, m_head = 0;
};
} // namespace Net

#endif // SOCKET_BUFFER_H_