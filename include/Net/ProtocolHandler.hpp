#ifndef PROTOCOLHANDLER_H_
#define PROTOCOLHANDLER_H_

#include "../Net/IProtocol.hpp"
#include "../Net/Socket.hpp"

#include <string>
#include <map>

#include <cassert>

namespace Net {
typedef IProtocol* IProtocolPtr;

class ProtocolHandler final {
public:
	enum MessageStatus { kNotReady, kUnknownOpcode, kSuccess };

	ProtocolHandler();

	~ProtocolHandler();

	ProtocolHandler(const ProtocolHandler&) = delete;
	ProtocolHandler& operator=(const ProtocolHandler&) = delete;

	IProtocolPtr GetProtocol(std::string name);
	bool IsValidBlock(uint8_t type);
	std::string GetBlockNameByType(uint8_t type);

	void RegisterProtocol(std::string name, IProtocolPtr protocol);
	MessageStatus HandleMessage(Net::Client* client) const;

private:
	std::map<std::string, IProtocolPtr> m_protocols;
};
} // namespace Net

#endif // PROTOCOLHANDLER_H_
