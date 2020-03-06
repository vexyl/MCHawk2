#include "Buffer.hpp"

BufferReader::BufferReader(const uint8_t* buffer, size_t bufferSize) : m_buffer(nullptr), m_bufferSize(bufferSize), m_head(0)
{
	if (buffer == nullptr || bufferSize == 0) {
		std::cerr << "Failed to create BufferReader: nullptr" << std::endl;
		std::exit(1);
	}

	m_buffer = static_cast<uint8_t*>(std::malloc(m_bufferSize));
	if (m_buffer == nullptr) {
		std::cerr << "Failed to create BufferReader: nullptr" << std::endl;
		std::exit(1);
	}

	std::memcpy(m_buffer, buffer, bufferSize);
}

BufferReader::~BufferReader()
{
	std::free(m_buffer);
}

void BufferReader::Skip(unsigned bytes)
{
	if (m_head + bytes > m_bufferSize) {
		std::cerr << "Failed skip bytes in buffer: max capacity" << std::endl;
		std::exit(1);
	}

	m_head += bytes;
}

void BufferReader::ReadUInt8(uint8_t& inData)
{
	Read(&inData, sizeof(inData));
}

void BufferReader::ReadUInt16(uint16_t& inData)
{
	Read(&inData, sizeof(inData));
}

void BufferReader::ReadInt16(int16_t& inData)
{
	Read(&inData, sizeof(inData));
}

void BufferReader::ReadUInt32(uint32_t& inData)
{
	Read(&inData, sizeof(inData));
}

void BufferReader::ReadString64(uint8_t(&inData)[64])
{
	Read(inData, 64);
}