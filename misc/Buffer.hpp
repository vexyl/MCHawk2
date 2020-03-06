#ifndef BUFFER_H_
#define BUFFER_H_

#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <iostream>

class BufferReader final {
public:
	BufferReader(const uint8_t* buffer, size_t bufferSize);

	~BufferReader();

	BufferReader(const BufferReader&) = delete;
	BufferReader& operator=(const BufferReader&) = delete;

	void Skip(unsigned bytes);

	void ReadUInt8(uint8_t& inData);
	void ReadUInt16(uint16_t& inData);
	void ReadInt16(int16_t& inData);
	void ReadUInt32(uint32_t& inData);
	void ReadString64(uint8_t(&inData)[64]);

private:
	uint8_t* m_buffer = nullptr;
	size_t m_bufferSize = 0, m_head = 0;

	void Read(void* outData, size_t outDataSize)
	{
		if (m_head + outDataSize > m_bufferSize) {
			std::cerr << "Failed to read from buffer: max capacity" << std::endl;
			std::exit(1);
		}

		std::memcpy(outData, m_buffer + m_head, outDataSize);
		m_head += outDataSize;
	}
};

class BufferWriter final {
public:
	BufferWriter(size_t bufferSize) : m_buffer(nullptr), m_bufferSize(bufferSize), m_head(0) {}

	~BufferWriter()
	{
		std::free(m_buffer);
	}

	const uint8_t* GetReadOnlyBufferPtr() const { return m_buffer; }
	size_t GetBufferSize() const { return m_bufferSize; }

	void WriteUInt8(uint8_t inData)
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

	void WriteUInt32(uint32_t inData)
	{
		Write(&inData, sizeof(inData));
	}

	void WriteString64(const uint8_t (&inData)[64])
	{
		Write(inData, 64);
	}

private:
	uint8_t* m_buffer = nullptr;
	size_t m_bufferSize = 0, m_head = 0;

	void Write(const void* inData, size_t inDataSize)
	{
		if (m_head + inDataSize > m_bufferSize) {
			std::cerr << "Failed to write to buffer: max capacity" << std::endl;
			std::exit(1);
		}

		if (m_buffer == nullptr) {
			m_buffer = static_cast<uint8_t*>(std::malloc(m_bufferSize));
			if (m_buffer == nullptr) {
				std::cerr << "Failed to write to buffer: not enough memory" << std::endl;
				std::exit(1);
			}
		}

		std::memcpy(m_buffer + m_head, inData, inDataSize);
		m_head += inDataSize;
	}
};

#endif // BUFFER_H_