#ifndef PACKET_H_
#define PACKET_H_

#include "../Utils/BufferStream.hpp"

#include <memory>

#include <cstdint>
#include <cstring>

namespace Net {
class Packet {
public:
	Packet() = default;
	Packet(uint8_t opcode, size_t packetSize) : m_opcode(opcode), m_packetSize(packetSize) {}

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
	std::unique_ptr<uint8_t[]> buffer;

	PartialPacket(void* inData, size_t inSize) : Packet(0, inSize)
	{
		buffer = std::make_unique<uint8_t[]>(m_packetSize);
		std::memcpy(buffer.get(), inData, inSize);
	}

	virtual void Deserialize(Utils::BufferStream& reader) override {}

	virtual std::unique_ptr<Utils::BufferStream> Serialize() override
	{
		auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);
		writer->Write(buffer.get(), m_packetSize);
		return std::move(writer);
	}
};
} // namespace Net

#endif // PACKET_H_
