#ifndef BUFFERSTREAM_H_
#define BUFFERSTREAM_H_

#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <iostream>

#include "MCString.hpp"

namespace Net {
	class Client;
}

namespace Utils {
class BufferStream final {
public:
	BufferStream(const uint8_t* buffer, size_t bufferSize) : m_bufferSize(bufferSize), m_readHead(0)
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

	BufferStream(size_t bufferSize) : m_bufferSize(bufferSize), m_readHead(0)
	{
		if (bufferSize == 0) {
			std::cerr << "Failed to create BufferStream: size 0" << std::endl;
			std::exit(1);
		}
	}

	~BufferStream()
	{
		std::free(m_buffer);
	}

	BufferStream(const BufferStream&) = delete;
	BufferStream& operator=(const BufferStream&) = delete;

	const uint8_t* GetReadOnlyBufferPtr() const { return m_buffer; }
	uint8_t* GetBufferPtr() { return m_buffer; }

	size_t GetBufferSize() const { return m_bufferSize; }

	void AllocateIfNeeded()
	{
		if (m_buffer == nullptr) {
			m_buffer = static_cast<uint8_t*>(std::malloc(m_bufferSize));
			if (m_buffer == nullptr) {
				std::cerr << "Failed to write to buffer: not enough memory" << std::endl;
				std::exit(1);
			}
		}
	}

	void Skip(unsigned bytes)
	{
		if (m_readHead + bytes > m_bufferSize) {
			std::cerr << "Failed skip bytes in buffer: max capacity" << std::endl;
			std::exit(1);
		}

		m_readHead += bytes;
	}

	void ReadUInt8(uint8_t& inData)
	{
		Read(&inData, sizeof(inData));
	}

	void ReadInt8(int8_t& inData)
	{
		Read(&inData, sizeof(inData));
	}

	void ReadUInt16(uint16_t& inData)
	{
		Read(&inData, sizeof(inData));
	}

	void ReadInt16(int16_t& inData)
	{
		Read(&inData, sizeof(inData));
	}

	void ReadInt32(int32_t& inData)
	{
		Read(&inData, sizeof(inData));
	}

	void ReadUInt32(uint32_t& inData)
	{
		Read(&inData, sizeof(inData));
	}

	void ReadString64(uint8_t(&inData)[64])
	{
		Read(inData, 64);
	}

	void ReadMCString(MCString& str)
	{
		ReadString64(str.m_data);
		str.Sanitize();
	}

	void WriteUInt8(uint8_t inData)
	{
		Write(&inData, sizeof(inData));
	}

	void WriteInt8(int8_t inData)
	{
		Write(&inData, sizeof(inData));
	}

	void WriteUInt16(uint16_t inData)
	{
		Write(&inData, sizeof(inData));
	}

	void WriteInt16(int16_t inData)
	{
		Write(&inData, sizeof(inData));
	}

	void WriteInt32(int32_t inData)
	{
		Write(&inData, sizeof(inData));
	}

	void WriteUInt32(uint32_t inData)
	{
		Write(&inData, sizeof(inData));
	}

	void WriteString64(const uint8_t(&inData)[64])
	{
		Write(inData, 64);
	}

	void WriteMCString(const MCString& str)
	{
		WriteString64(str.m_data);
	}

	void WriteArray1024(const uint8_t(&inData)[1024])
	{
		Write(inData, 1024);
	}

private:
	friend Net::Client;

	uint8_t* m_buffer = nullptr;
	size_t m_bufferSize = 0, m_readHead, m_writeHead = 0;

	void Read(void* outData, size_t outDataSize)
	{
		if (m_readHead + outDataSize > m_bufferSize) {
			std::cerr << "Failed to read from buffer: max capacity" << std::endl;
			std::exit(1);
		}

		std::memcpy(outData, m_buffer + m_readHead, outDataSize);
		m_readHead += outDataSize;
	}

	void Write(const void* inData, size_t inDataSize)
	{
		if (m_writeHead + inDataSize > m_bufferSize) {
			std::cerr << "Failed to write to buffer: max capacity" << std::endl;
			std::exit(1);
		}

		AllocateIfNeeded();

		std::memcpy(m_buffer + m_writeHead, inData, inDataSize);
		m_writeHead += inDataSize;
	}
};
} // namespace Utils

#endif // BUFFERSTREAM_H_