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
} // namespace Net

#endif // PACKET_H_
