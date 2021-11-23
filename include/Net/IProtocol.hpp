#ifndef IPROTOCOL_H_
#define IPROTOCOL_H_

#include "../Utils/BufferStream.hpp"

#include <functional>
#include <cstdint>

namespace Net {
class Client;

// FIXME: remove this after cleaning up protocols
struct OpcodeHandler final {
	std::function<void(Client*, Utils::BufferStream&)> handler;
	size_t packetSize = 0;
};

class IProtocol {
public:
	virtual size_t GetPacketSize(uint8_t opcode) const = 0;
	virtual bool HandleOpcode(uint8_t opcode, Client* client, Utils::BufferStream& reader) const = 0;
	virtual bool IsValidBlock(uint8_t type) const = 0;
	virtual std::string GetBlockNameByType(uint8_t type) const = 0;
};
} // namespace Net

#endif // IPROTOCOL_H_
