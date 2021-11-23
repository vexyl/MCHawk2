#include "../../include/Net/ProtocolHandler.hpp"
#include "../../include/Net/ClassicProtocol.hpp"
#include "../../include/Net/ExtendedProtocol.hpp"
#include "../../include/Net/Client.hpp"

using namespace Net;

ProtocolHandler::ProtocolHandler()
{
	RegisterProtocol("ClassicProtocol", new ClassicProtocol());
	RegisterProtocol("ExtendedProtocol", new ExtendedProtocol());
}

ProtocolHandler::~ProtocolHandler()
{
	for (auto& entry : m_protocols)
		delete entry.second;
}

IProtocolPtr ProtocolHandler::GetProtocol(std::string name)
{
	auto protocolEntry = m_protocols.find(name);
	if (protocolEntry == m_protocols.end())
		return nullptr;

	return protocolEntry->second;
}

bool ProtocolHandler::IsValidBlock(uint8_t type)
{
	bool result = false;
	for (const auto& entry : m_protocols) {
		result = entry.second->IsValidBlock(type);
		if (result)
			break;
	}

	return result;
}

std::string ProtocolHandler::GetBlockNameByType(uint8_t type)
{
	std::string result;
	for (const auto& entry : m_protocols) {
		result = entry.second->GetBlockNameByType(type);
		if (result != "")
			break;
	}

	return result;
}

void ProtocolHandler::RegisterProtocol(std::string name, IProtocolPtr protocol)
{
	if (m_protocols.find(name) != m_protocols.end()) {
		std::cerr << "Failed to add protocol: already exists" << std::endl;
		exit(1);
	}

	m_protocols.insert(std::make_pair(name, std::move(protocol)));
}

ProtocolHandler::MessageStatus ProtocolHandler::HandleMessage(Net::Client* client) const
{
	Net::Socket* socket = client->GetSocket();
	uint8_t opcode = socket->PeekFirstByte();
	bool handledOpcode = false;

	for (const auto& entry : m_protocols) {
		size_t packetSize = entry.second->GetPacketSize(opcode);
		if (packetSize == 0)
			continue;

		Utils::BufferStream reader(packetSize);

		if (socket->Receive(reader) < 0)
			return MessageStatus::kNotReady;

		handledOpcode = entry.second->HandleOpcode(opcode, client, reader);
		if (handledOpcode) {
			//std::cout << "Opcode " << static_cast<int>(opcode) << " handled by protocol: " << entry.first << "\n" << std::endl;
			return MessageStatus::kSuccess;
		}
	}

	return MessageStatus::kUnknownOpcode;
}