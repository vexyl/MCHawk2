#ifndef ISERVER
#define ISERVER

#include "Net/ProtocolHandler.hpp"

#include <string>

#include <cstdint>

class IServer {
public:
	virtual uint8_t GetCPEEntryVersion(std::string name) const = 0;
	virtual bool IsValidBlock(uint8_t blockType) const = 0;
	virtual std::string GetBlockNameByType(uint8_t blockType) const = 0;
};

#endif // ISERVER_HELPER
