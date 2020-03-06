#include "../../include/Net/ClassicProtocol.hpp"
#include "../../include/Server.hpp"

using namespace Net;

// Are the sizes necessary here? Packet has GetSize()
ClassicProtocol::ClassicProtocol()
{
	// Default callbacks
	m_defaultPacketHandlers.insert(std::make_pair(
		ClientOpcodes::kAuthentication,
		OpcodeHandler{
			[&](Client* client, Utils::BufferStream& reader) { AuthenticationOpcodeHandler(client, reader); },
			131 /* packet size */
		}
	));

	m_defaultPacketHandlers.insert(std::make_pair(
		ClientOpcodes::kSetBlock,
		OpcodeHandler{
			[&](Client* client, Utils::BufferStream& reader) { SetBlockOpcodeHandler(client, reader); },
			9 /* packet size */
		}
	));

	m_defaultPacketHandlers.insert(std::make_pair(
		ClientOpcodes::kPositionOrientation,
		OpcodeHandler{
			[&](Client* client, Utils::BufferStream& reader) { PositionOrientationOpcodeHandler(client, reader); },
			10 /* packet size */
		}
	));

	m_defaultPacketHandlers.insert(std::make_pair(
		ClientOpcodes::kMessage,
		OpcodeHandler{
			[&](Client* client, Utils::BufferStream& reader) { MessageOpcodeHandler(client, reader); },
			66 /* packet size */
		}
	));
}

size_t ClassicProtocol::GetPacketSize(uint8_t opcode) const
{
	auto packetHandlerEntry = m_defaultPacketHandlers.find(static_cast<ClientOpcodes>(opcode));
	if (packetHandlerEntry != m_defaultPacketHandlers.end())
		return packetHandlerEntry->second.packetSize;
	else
		return 0;
}

bool ClassicProtocol::HandleOpcode(uint8_t opcode, Client* client, Utils::BufferStream& reader) const
{
	auto packetHandlerEntry = m_defaultPacketHandlers.find(static_cast<ClientOpcodes>(opcode));
	if (packetHandlerEntry != m_defaultPacketHandlers.end()) {
		packetHandlerEntry->second.handler(client, reader);
		return true;
	}

	return false;
}

void ClassicProtocol::AuthenticationOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	std::cout << "[AuthenticationOpcodeHandler]" << std::endl;

	AuthenticationPacket packet;
	packet.Deserialize(reader);

	authEvents.Trigger(client, packet);
}

void ClassicProtocol::SetBlockOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	std::cout << "[SetBlockOpcodeHandler]" << std::endl;

	SetBlockPacket packet;
	packet.Deserialize(reader);

	setBlockEvents.Trigger(client, packet);

	std::cout
		<< "SetBlock type=" << static_cast<unsigned>(packet.type)
		<< ", mode=" << static_cast<unsigned>(packet.mode)
		<< " @ ("
		<< static_cast<short>(packet.x) << ", "
		<< static_cast<short>(packet.y) << ", "
		<< static_cast<short>(packet.z) << ")"
		<< std::endl;
}

void ClassicProtocol::PositionOrientationOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	//std::cout << "[PositionOrientationOpcodeHandler]" << std::endl;

	PositionOrientationPacket packet;
	packet.Deserialize(reader);

	positionOrientationEvents.Trigger(client, packet);

	/*std::cout
		<< "PositionOrientation pid=" << static_cast<signed>(packet.pid)
		<< " @ ("
		<< static_cast<short>(packet.x) << ", "
		<< static_cast<short>(packet.y) << ", "
		<< static_cast<short>(packet.z) << ")"
		<< ", yaw=" << static_cast<unsigned>(packet.yaw)
		<< ", pitch=" << static_cast<unsigned>(packet.pitch)
		<< std::endl;*/
}

void ClassicProtocol::MessageOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	std::cout << "[MessageOpcodeHandler]" << std::endl;

	MessagePacket packet;
	packet.Deserialize(reader);

	messageEvents.Trigger(client, packet);
}

std::shared_ptr<ClassicProtocol::PositionOrientationPacket> ClassicProtocol::MakePositionOrientationPacket(int8_t pid, int16_t x, int16_t y, int16_t z, uint8_t yaw, uint8_t pitch)
{
	return std::make_shared<ClassicProtocol::PositionOrientationPacket>(pid, x, y, z, yaw, pitch);
}

std::shared_ptr<ClassicProtocol::OrientationPacket> ClassicProtocol::MakeOrientationPacket(int8_t pid, uint8_t yaw, uint8_t pitch)
{
	return std::make_shared<ClassicProtocol::OrientationPacket>(pid, yaw, pitch);
}

std::shared_ptr<ClassicProtocol::DespawnPacket> ClassicProtocol::MakeDespawnPacket(int8_t pid)
{
	return std::make_shared<ClassicProtocol::DespawnPacket>(pid);
}

std::shared_ptr<ClassicProtocol::MessagePacket> ClassicProtocol::MakeMessagePacket(uint8_t flag, Utils::MCString message)
{
	return std::make_shared<ClassicProtocol::MessagePacket>(flag, message);
}

std::shared_ptr<ClassicProtocol::ServerIdentificationPacket> ClassicProtocol::MakeServerIdentificationPacket(uint8_t version, Utils::MCString name, Utils::MCString motd, uint8_t userType)
{
	return std::make_shared<ClassicProtocol::ServerIdentificationPacket>(version, name, motd, userType);
}

std::shared_ptr<ClassicProtocol::SpawnPlayerPacket> ClassicProtocol::MakeSpawnPlayerPacket(int8_t pid, Utils::MCString name, int16_t x, int16_t y, int16_t z, uint8_t yaw, uint8_t pitch)
{
	return std::make_shared<ClassicProtocol::SpawnPlayerPacket>(pid, name, x, y, z, yaw, pitch);
}

std::shared_ptr<ClassicProtocol::LevelInitializePacket> ClassicProtocol::MakeLevelInitializePacket()
{
	return std::make_shared<ClassicProtocol::LevelInitializePacket>();
}

// TODO: LevelDataChunkPacket

std::shared_ptr<ClassicProtocol::LevelFinalizePacket> ClassicProtocol::MakeLevelFinalizePacket(int16_t x, int16_t y, int16_t z)
{
	return std::make_shared<ClassicProtocol::LevelFinalizePacket>(x, y, z);
}

std::shared_ptr<ClassicProtocol::SetBlock2Packet> ClassicProtocol::MakeSetBlock2Packet(int16_t x, int16_t y, int16_t z, uint8_t type)
{
	return std::make_shared<ClassicProtocol::SetBlock2Packet>(x, y, z, type);
}