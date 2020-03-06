#ifndef IPROTOCOL_H_
#define IPROTOCOL_H_

#include "../../include/Net/IProtocol.hpp"
#include "../../include/Utils/BufferStream.hpp"

#include <cstdint>

namespace Net {
class Client;

class IProtocol {
public:
	virtual size_t GetPacketSize(uint8_t opcode) const = 0;
	virtual bool HandleOpcode(uint8_t opcode, Client* client, Utils::BufferStream& reader) const = 0;
};
} // namespace Net

#endif // IPROTOCOL_H_