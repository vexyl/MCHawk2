#include "../../include/Net/ClassicProtocol.hpp"
#include "../../include/Server.hpp"

using namespace Net;

std::map<uint8_t, std::string> ClassicProtocol::blockTypes = {
	{ kAir, "air" },
	{ kStone, "stone" },
	{ kGrass, "grass" },
	{ kDirt, "dirt" },
	{ kCobblestone, "cobblestone" },
	{ kWoodPlanks, "wood planks" },
	{ kSapling, "sapling" },
	{ kBedrock, "bedrock" },
	{ kFlowingWater, "flowing water" },
	{ kStationaryWater, "stationary water" },
	{ kFlowingLava, "flowing lava" },
	{ kStationaryLava, "stationary lava" },
	{ kSand, "sand" },
	{ kGravel, "gravel" },
	{ kGoldOre, "gold ore" },
	{ kCoalOre, "coal ore" },
	{ kWood, "wood" },
	{ kLeaves, "leaves" },
	{ kSponge, "sponge" },
	{ kGlass, "glass" },
	{ kRedCloth, "red cloth" },
	{ kOrangeCloth, "orange cloth" },
	{ kYellowCloth, "yellow cloth" },
	{ kLimeCloth, "lime cloth" },
	{ kGreenCloth, "green cloth" },
	{ kAquaGreenCloth, "aqua cloth" },
	{ kCyanCloth, "cyan cloth" },
	{ kBlueCloth, "blue cloth" },
	{ kPurpleCloth, "purple cloth" },
	{ kIndigoCloth, "indigo cloth" },
	{ kVioletCloth, "violet cloth" },
	{ kMagentaCloth, "magenta cloth" },
	{ kPinkCloth, "pink cloth" },
	{ kBlackCloth, "black cloth" },
	{ kGrayCloth, "gray cloth" },
	{ kWhiteCloth, "white cloth" },
	{ kDandelion, "dandelion" },
	{ kRose, "rose" },
	{ kBrownMushroom, "brown mushroom" },
	{ kRedMushroom, "red mushroom" },
	{ kGoldBlock, "gold block" },
	{ kIronBlock, "iron block" },
	{ kDoubleSlab, "double slab" },
	{ kSlab, "slab" },
	{ kBricks, "bricks" },
	{ kTNT, "tnt" },
	{ kBookshelf, "bookshelf" },
	{ kMossStone, "moss stone" },
	{ kObsidian, "obsidian" }
};

ClassicProtocol::ClassicProtocol()
{
	// Default callbacks
	m_defaultPacketHandlers.insert(std::make_pair(
		Opcodes::kAuthentication,
		OpcodeHandler{
			[this](Client* client, Utils::BufferStream& reader) { AuthenticationOpcodeHandler(client, reader); },
			kAuthenticationSize /* packet size */
		}
	));

	m_defaultPacketHandlers.insert(std::make_pair(
		Opcodes::kSetBlock,
		OpcodeHandler{
			[this](Client* client, Utils::BufferStream& reader) { SetBlockOpcodeHandler(client, reader); },
			kSetBlockSize /* packet size */
		}
	));

	m_defaultPacketHandlers.insert(std::make_pair(
		Opcodes::kPositionOrientation,
		OpcodeHandler{
			[this](Client* client, Utils::BufferStream& reader) { PositionOrientationOpcodeHandler(client, reader); },
			kPositionOrientationSize /* packet size */
		}
	));

	m_defaultPacketHandlers.insert(std::make_pair(
		Opcodes::kMessage,
		OpcodeHandler{
			[this](Client* client, Utils::BufferStream& reader) { MessageOpcodeHandler(client, reader); },
			kMessageSize /* packet size */
		}
	));
}

size_t ClassicProtocol::GetPacketSize(uint8_t opcode) const
{
	auto packetHandlerEntry = m_defaultPacketHandlers.find(static_cast<Opcodes>(opcode));
	if (packetHandlerEntry != m_defaultPacketHandlers.end())
		return packetHandlerEntry->second.packetSize;
	else
		return 0;
}

bool ClassicProtocol::HandleOpcode(uint8_t opcode, Client* client, Utils::BufferStream& reader) const
{
	auto packetHandlerEntry = m_defaultPacketHandlers.find(static_cast<Opcodes>(opcode));
	if (packetHandlerEntry != m_defaultPacketHandlers.end()) {
		packetHandlerEntry->second.handler(client, reader);
		return true;
	}

	return false;
}

void ClassicProtocol::AuthenticationOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	//std::cout << "[AuthenticationOpcodeHandler]" << std::endl;

	if (onAuthenticationCallback == nullptr)
		return;

	AuthenticationPacket packet;
	packet.Deserialize(reader);

	onAuthenticationCallback(client, packet);
}

void ClassicProtocol::SetBlockOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	//std::cout << "[SetBlockOpcodeHandler]" << std::endl;
	if (onSetBlockCallback == nullptr)
		return;

	SetBlockPacket packet;
	packet.Deserialize(reader);

	onSetBlockCallback(client, packet);
}

void ClassicProtocol::PositionOrientationOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	//std::cout << "[PositionOrientationOpcodeHandler]" << std::endl;

	if (onPositionOrientationCallback == nullptr)
		return;

	PositionOrientationPacket packet;
	packet.Deserialize(reader);

	/*std::cout
		<< "PositionOrientation pid=" << static_cast<signed>(packet.pid)
		<< " @ ("
		<< static_cast<short>(packet.x) << ", "
		<< static_cast<short>(packet.y) << ", "
		<< static_cast<short>(packet.z) << ")"
		<< ", yaw=" << static_cast<unsigned>(packet.yaw)
		<< ", pitch=" << static_cast<unsigned>(packet.pitch)
		<< std::endl;*/

	onPositionOrientationCallback(client, packet);
}

void ClassicProtocol::MessageOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	//std::cout << "[MessageOpcodeHandler]" << std::endl;

	if (onMessageCallback == nullptr)
		return;

	MessagePacket packet;
	packet.Deserialize(reader);

	onMessageCallback(client, packet);
}

/* BEGIN AUTOGENERATED CODE SECTION */
std::shared_ptr<Packet> ClassicProtocol::MakeSetBlockPacket(
	int16_t x,
	int16_t y,
	int16_t z,
	uint8_t mode,
	uint8_t type
)
{
	return std::make_shared<ClassicProtocol::SetBlockPacket>(
		x,
		y,
		z,
		mode,
		type
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakePositionOrientationPacket(
	int8_t pid,
	int16_t x,
	int16_t y,
	int16_t z,
	uint8_t yaw,
	uint8_t pitch
)
{
	return std::make_shared<ClassicProtocol::PositionOrientationPacket>(
		pid,
		x,
		y,
		z,
		yaw,
		pitch
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeOrientationPacket(
	int8_t pid,
	uint8_t yaw,
	uint8_t pitch
)
{
	return std::make_shared<ClassicProtocol::OrientationPacket>(
		pid,
		yaw,
		pitch
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeDespawnPacket(
	int8_t pid
)
{
	return std::make_shared<ClassicProtocol::DespawnPacket>(
		pid
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeMessagePacket(
	uint8_t flag,
	Utils::MCString message
)
{
	return std::make_shared<ClassicProtocol::MessagePacket>(
		flag,
		message
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeServerIdentificationPacket(
	uint8_t version,
	Utils::MCString name,
	Utils::MCString motd,
	uint8_t userType
)
{
	return std::make_shared<ClassicProtocol::ServerIdentificationPacket>(
		version,
		name,
		motd,
		userType
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeLevelInitializePacket(
)
{
	return std::make_shared<ClassicProtocol::LevelInitializePacket>(
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeLevelDataChunkPacket(
	uint16_t chunkLength,
	uint8_t chunkData[1024],
	uint8_t percent
)
{
	return std::make_shared<ClassicProtocol::LevelDataChunkPacket>(
		chunkLength,
		chunkData,
		percent
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeLevelFinalizePacket(
	int16_t x,
	int16_t y,
	int16_t z
)
{
	return std::make_shared<ClassicProtocol::LevelFinalizePacket>(
		x,
		y,
		z
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeSetBlock2Packet(
	int16_t x,
	int16_t y,
	int16_t z,
	uint8_t type
)
{
	return std::make_shared<ClassicProtocol::SetBlock2Packet>(
		x,
		y,
		z,
		type
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeSpawnPlayerPacket(
	int8_t pid,
	Utils::MCString name,
	int16_t x,
	int16_t y,
	int16_t z,
	uint8_t yaw,
	uint8_t pitch
)
{
	return std::make_shared<ClassicProtocol::SpawnPlayerPacket>(
		pid,
		name,
		x,
		y,
		z,
		yaw,
		pitch
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeUserTypePacket(
	uint8_t type
)
{
	return std::make_shared<ClassicProtocol::UserTypePacket>(
		type
		);
}

std::shared_ptr<Packet> ClassicProtocol::MakeDisconnectPlayerPacket(
	Utils::MCString reason
)
{
	return std::make_shared<ClassicProtocol::DisconnectPlayerPacket>(
		reason
		);
}
/* END AUTOGENERATED CODE SECTION */
