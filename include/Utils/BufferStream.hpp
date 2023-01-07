#ifndef BUFFERSTREAM_H_
#define BUFFERSTREAM_H_

#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <iostream>

#include "MCString.hpp"

namespace Net {
	class Client;
	class PartialPacket;
}

namespace Utils {
class BufferStream final {
public:
	BufferStream(const uint8_t* buffer, size_t bufferSize);
	BufferStream(size_t bufferSize);

	~BufferStream();

	BufferStream(const BufferStream&) = delete;
	BufferStream(BufferStream&&) = delete;
	BufferStream& operator=(const BufferStream&) = delete;
	BufferStream& operator=(BufferStream&&) = delete;

	const uint8_t* GetReadOnlyBufferPtr() const { return m_buffer; }
	uint8_t* GetBufferPtr() { return m_buffer; }
	size_t GetBufferSize() const { return m_bufferSize; }

	void AllocateIfNeeded();
	void Skip(unsigned bytes);

	void ReadUInt8(uint8_t& inData);
	void ReadInt8(int8_t& inData);
	void ReadUInt16(uint16_t& inData);
	void ReadInt16(int16_t& inData);
	void ReadInt32(int32_t& inData);
	void ReadUInt32(uint32_t& inData);
	void ReadString64(uint8_t(&inData)[64]);
	void ReadMCString(MCString& str);

	void WriteUInt8(uint8_t inData);
	void WriteInt8(int8_t inData);
	void WriteUInt16(uint16_t inData);
	void WriteInt16(int16_t inData);
	void WriteInt32(int32_t inData);
	void WriteUInt32(uint32_t inData);
	void WriteString64(const uint8_t(&inData)[64]);
	void WriteMCString(const MCString& str);
	void WriteArray1024(const uint8_t(&inData)[1024]);

	void Read(void* outData, size_t outDataSize);
	void Write(const void* inData, size_t inDataSize);
private:
	uint8_t* m_buffer = nullptr;
	size_t m_bufferSize = 0, m_readHead, m_writeHead = 0;
};
} // namespace Utils

#endif // BUFFERSTREAM_H_
