#ifndef PACKET_H_
#define PACKET_H_

#include "../Utils/BufferStream.hpp"

#include <memory>

#include <cstdint>

namespace Net {
class Packet {
public:
	Packet(uint8_t opcode, size_t packetSize) : m_opcode(opcode), m_packetSize(packetSize) {}
	Packet() {}

	virtual ~Packet() {}

	Packet(const Packet&) = delete;
	Packet& operator=(const Packet&) = delete;

	size_t GetSize() const { return m_packetSize; }

	virtual void Deserialize(Utils::BufferStream& reader) = 0;
	virtual std::unique_ptr<Utils::BufferStream> Serialize() = 0;

protected:
	size_t m_packetSize = 0;
	uint8_t m_opcode = 0;
};

class PartialPacket final : public Net::Packet
{
public:
	uint8_t* buffer;

	PartialPacket(void* inData, size_t inSize) : Packet(0, inSize)
	{
		buffer = new uint8_t[m_packetSize];
		memcpy(buffer, inData, inSize);
	}

	~PartialPacket()
	{
		delete[] buffer;
	}

	virtual void Deserialize(Utils::BufferStream& reader) override
	{
	}

	virtual std::unique_ptr<Utils::BufferStream> Serialize() override
	{
		auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);
		writer->Write(buffer, m_packetSize);
		return std::move(writer);
	}
};
} // namespace Net

#endif // PACKET_H_
