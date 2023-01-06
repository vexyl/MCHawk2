#include "../../include/Utils/BufferStream.hpp"

using namespace Utils;

BufferStream::BufferStream(const uint8_t* buffer, size_t bufferSize) : m_bufferSize(bufferSize), m_readHead(0)
{
	if (buffer == nullptr || bufferSize == 0) {
		std::cerr << "Failed to create BufferStream: nullptr" << std::endl;
		std::exit(1);
	}

	m_buffer = static_cast<uint8_t*>(std::malloc(m_bufferSize));
	if (m_buffer == nullptr) {
		std::cerr << "Failed to create BufferStream: nullptr" << std::endl;
		std::exit(1);
	}

	std::memcpy(m_buffer, buffer, bufferSize);
}

BufferStream::BufferStream(size_t bufferSize) : m_bufferSize(bufferSize), m_readHead(0)
{
	if (bufferSize == 0) {
		std::cerr << "Failed to create BufferStream: size 0" << std::endl;
		std::exit(1);
	}
}

BufferStream::~BufferStream()
{
	std::free(m_buffer);
}

void BufferStream::AllocateIfNeeded()
{
	if (m_buffer == nullptr) {
		m_buffer = static_cast<uint8_t*>(std::malloc(m_bufferSize));
		if (m_buffer == nullptr) {
			std::cerr << "Failed to write to buffer: not enough memory" << std::endl;
			std::exit(1);
		}
	}
}

void BufferStream::Skip(unsigned bytes)
{
	if (m_readHead + bytes > m_bufferSize) {
		std::cerr << "Failed skip bytes in buffer: max capacity" << std::endl;
		std::exit(1);
	}

	m_readHead += bytes;
}

void BufferStream::ReadUInt8(uint8_t& inData)
{
	Read(&inData, sizeof(inData));
}

void BufferStream::ReadInt8(int8_t& inData)
{
	Read(&inData, sizeof(inData));
}

void BufferStream::ReadUInt16(uint16_t& inData)
{
	Read(&inData, sizeof(inData));
}

void BufferStream::ReadInt16(int16_t& inData)
{
	Read(&inData, sizeof(inData));
}

void BufferStream::ReadInt32(int32_t& inData)
{
	Read(&inData, sizeof(inData));
}

void BufferStream::ReadUInt32(uint32_t& inData)
{
	Read(&inData, sizeof(inData));
}

void BufferStream::ReadString64(uint8_t(&inData)[64])
{
	Read(inData, 64);
}

void BufferStream::ReadMCString(MCString& str)
{
	ReadString64(str.m_data);
	str.Sanitize();
}

void BufferStream::WriteUInt8(uint8_t inData)
{
	Write(&inData, sizeof(inData));
}

void BufferStream::WriteInt8(int8_t inData)
{
	Write(&inData, sizeof(inData));
}

void BufferStream::WriteUInt16(uint16_t inData)
{
	Write(&inData, sizeof(inData));
}

void BufferStream::WriteInt16(int16_t inData)
{
	Write(&inData, sizeof(inData));
}

void BufferStream::WriteInt32(int32_t inData)
{
	Write(&inData, sizeof(inData));
}

void BufferStream::WriteUInt32(uint32_t inData)
{
	Write(&inData, sizeof(inData));
}

void BufferStream::WriteString64(const uint8_t(&inData)[64])
{
	Write(inData, 64);
}

void BufferStream::WriteMCString(const MCString& str)
{
	WriteString64(str.m_data);
}

void BufferStream::WriteArray1024(const uint8_t(&inData)[1024])
{
	Write(inData, 1024);
}

void BufferStream::Read(void* outData, size_t outDataSize)
{
	if (m_readHead + outDataSize > m_bufferSize) {
		std::cerr << "Failed to read from buffer: max capacity" << std::endl;
		std::exit(1);
	}

	std::memcpy(outData, m_buffer + m_readHead, outDataSize);
	m_readHead += outDataSize;
}

void BufferStream::Write(const void* inData, size_t inDataSize)
{
	if (m_writeHead + inDataSize > m_bufferSize) {
		std::cerr << "Failed to write to buffer: max capacity" << std::endl;
		std::exit(1);
	}

	AllocateIfNeeded();

	std::memcpy(m_buffer + m_writeHead, inData, inDataSize);
	m_writeHead += inDataSize;
}