#include "../../include/Net/SocketBuffer.hpp"

using namespace Net;

SocketBuffer::SocketBuffer(size_t size) : m_bufferSize(size)
{
	m_buffer = new uint8_t[m_bufferSize];
}

SocketBuffer::~SocketBuffer()
{
	delete m_buffer;
}

// Writes as much data as possible
size_t SocketBuffer::Write(void* inData, size_t inDataSize)
{
	size_t byteCount = std::min(inDataSize, m_bufferSize - m_head);

	std::memcpy(m_buffer + m_head, inData, byteCount);
	m_head += byteCount;

	return byteCount;
}

bool SocketBuffer::Read(void* outData, size_t outDataSize)
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

bool SocketBuffer::Read(Utils::BufferStream& bufferStream)
{
	bufferStream.AllocateIfNeeded();
	return Read(bufferStream.GetBufferPtr(), bufferStream.GetBufferSize());
}