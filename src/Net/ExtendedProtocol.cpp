#include "../../include/Net/ExtendedProtocol.hpp"
#include "../../include/Server.hpp"

using namespace Net;

std::map<uint8_t, std::string> ExtendedProtocol::blockTypes = {
	{ kCobblestoneSlab, "cobblestone slab" },
	{ kRope, "rope" },
	{ kSandstone, "sandstone" },
	{ kSnow, "snow" },
	{ kFire, "fire" },
	{ kLightPinkWool, "light pink wool" },
	{ kForestGreenWool, "forest green wool" },
	{ kBrownWool, "brown wool" },
	{ kDeepBlue, "deep blue" },
	{ kTurquoise, "turquoise" },
	{ kIce, "ice" },
	{ kCeramicTile, "ceramic tile" },
	{ kMagma, "magma" },
	{ kPillar, "pillar" },
	{ kCrate, "crate" },
	{ kStoneBrick, "stone brick" }
};

// Are the sizes necessary here? Packet has GetSize()
ExtendedProtocol::ExtendedProtocol()
{
	// Default callbacks
	m_defaultPacketHandlers.insert(std::make_pair(
		Opcodes::kExtInfo,
		OpcodeHandler{
			[&](Client* client, Utils::BufferStream& reader) { ExtInfoOpcodeHandler(client, reader); },
			kExtInfoSize /* packet size */
		}
	));

	// Default callbacks
	m_defaultPacketHandlers.insert(std::make_pair(
		Opcodes::kExtEntry,
		OpcodeHandler{
			[&](Client* client, Utils::BufferStream& reader) { ExtEntryOpcodeHandler(client, reader); },
			kExtEntrySize /* packet size */
		}
	));

	// Default callbacks
	m_defaultPacketHandlers.insert(std::make_pair(
		Opcodes::kCustomBlocks,
		OpcodeHandler{
			[&](Client* client, Utils::BufferStream& reader) { CustomBlocksOpcodeHandler(client, reader); },
			kCustomBlocksSize /* packet size */
		}
	));

	// Default callbacks
	m_defaultPacketHandlers.insert(std::make_pair(
		Opcodes::kPlayerClick,
		OpcodeHandler{
			[&](Client* client, Utils::BufferStream& reader) { PlayerClickOpcodeHandler(client, reader); },
			kPlayerClickSize /* packet size */
		}
	));
}


void ExtendedProtocol::ExtInfoOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	//std::cout << "[ExtInfo]" << std::endl;

	if (onExtInfoCallback == nullptr)
		return;

	ExtInfoPacket packet;
	packet.Deserialize(reader);

	onExtInfoCallback(client, packet);
}

void ExtendedProtocol::ExtEntryOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	//std::cout << "[ExtEntry]" << std::endl;

	if (onExtEntryCallback == nullptr)
		return;

	ExtEntryPacket packet;
	packet.Deserialize(reader);

	onExtEntryCallback(client, packet);
}

void ExtendedProtocol::CustomBlocksOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	//std::cout << "[CustomBlocks]" << std::endl;

	if (onCustomBlocksCallback == nullptr)
		return;

	CustomBlocksPacket packet;
	packet.Deserialize(reader);

	onCustomBlocksCallback(client, packet);
}

void ExtendedProtocol::PlayerClickOpcodeHandler(Client* client, Utils::BufferStream& reader)
{
	//std::cout << "[PlayerClick]" << std::endl;

	if (onPlayerClickCallback == nullptr)
		return;

	PlayerClickPacket packet;
	packet.Deserialize(reader);

	onPlayerClickCallback(client, packet);

	//std::cout << std::to_string(packet.action) << "," << std::to_string(packet.button) << "," << std::to_string(packet.targetBlockX) << ", " << std::to_string(packet.targetBlockY) << "," << std::to_string(packet.targetBlockZ) << " | " << std::to_string(packet.targetEntityID) << std::endl;
}

size_t ExtendedProtocol::GetPacketSize(uint8_t opcode) const
{
	auto packetHandlerEntry = m_defaultPacketHandlers.find(static_cast<Opcodes>(opcode));
	if (packetHandlerEntry != m_defaultPacketHandlers.end())
		return packetHandlerEntry->second.packetSize;
	else
		return 0;
}

bool ExtendedProtocol::HandleOpcode(uint8_t opcode, Client* client, Utils::BufferStream& reader) const
{
	auto packetHandlerEntry = m_defaultPacketHandlers.find(static_cast<Opcodes>(opcode));
	if (packetHandlerEntry != m_defaultPacketHandlers.end()) {
		packetHandlerEntry->second.handler(client, reader);
		return true;
	}

	return false;
}

std::shared_ptr<ExtendedProtocol::ExtInfoPacket> ExtendedProtocol::MakeExtInfoPacket(Utils::MCString appName, uint16_t extensionCount)
{
	return std::make_shared<ExtendedProtocol::ExtInfoPacket>(appName, extensionCount);
}

std::shared_ptr<ExtendedProtocol::ExtEntryPacket> ExtendedProtocol::MakeExtEntryPacket(Utils::MCString extName, uint32_t version)
{
	return std::make_shared<ExtendedProtocol::ExtEntryPacket>(extName, version);
}

std::shared_ptr<ExtendedProtocol::CustomBlocksPacket> ExtendedProtocol::MakeCustomBlocksPacket(uint8_t supportedLevel)
{
	return std::make_shared<ExtendedProtocol::CustomBlocksPacket>(supportedLevel);
}

std::shared_ptr<ExtendedProtocol::SetTextHotkeyPacket> ExtendedProtocol::MakeSetTextHotkeyPacket(Utils::MCString label, Utils::MCString action, uint32_t keyCode, uint8_t keyMods)
{
	return std::make_shared<ExtendedProtocol::SetTextHotkeyPacket>(label, action, keyCode, keyMods);
}

std::shared_ptr<ExtendedProtocol::ExtAddPlayerNamePacket> ExtendedProtocol::MakeExtAddPlayerNamePacket(uint16_t nameID, Utils::MCString playerName, Utils::MCString listName, Utils::MCString groupName, uint8_t groupRank)
{
	return std::make_shared<ExtendedProtocol::ExtAddPlayerNamePacket>(nameID, playerName, listName, groupName, groupRank);
}

std::shared_ptr<ExtendedProtocol::ExtAddEntity2Packet> ExtendedProtocol::MakeExtAddEntity2Packet(int8_t entityID, Utils::MCString inGameName, Utils::MCString skinName, int16_t spawnX, int16_t spawnY, int16_t spawnZ, uint8_t spawnYaw, uint8_t spawnPitch)
{
	return std::make_shared<ExtendedProtocol::ExtAddEntity2Packet>(entityID, inGameName, skinName, spawnX, spawnY, spawnZ, spawnYaw, spawnPitch);
}

std::shared_ptr<ExtendedProtocol::ChangeModelPacket> ExtendedProtocol::MakeChangeModelPacket(int8_t entityID, Utils::MCString modelName)
{
	return std::make_shared<ExtendedProtocol::ChangeModelPacket>(entityID, modelName);
}

std::shared_ptr<ExtendedProtocol::DefineBlockPacket> ExtendedProtocol::MakeDefineBlockPacket(
	uint8_t blockID,
	Utils::MCString name,
	uint8_t solidity,
	uint8_t movementSpeed,
	uint8_t topTextureID, uint8_t sideTextureID, uint8_t bottomTextureID,
	uint8_t transmitLight,
	uint8_t walkSound,
	uint8_t fullBright,
	uint8_t shape,
	uint8_t blockDraw,
	uint8_t fogDensity,
	uint8_t fogR, uint8_t fogG, uint8_t fogB)
{
	return std::make_shared<ExtendedProtocol::DefineBlockPacket>(
		blockID,
		name,
		solidity,
		movementSpeed,
		topTextureID, sideTextureID, bottomTextureID,
		transmitLight,
		walkSound,
		fullBright,
		shape,
		blockDraw,
		fogDensity,
		fogR, fogG, fogB
		);
}

std::shared_ptr<ExtendedProtocol::HoldThisPacket> ExtendedProtocol::MakeHoldThisPacket(uint8_t blockToHold, uint8_t preventChange)
{
	return std::make_shared<ExtendedProtocol::HoldThisPacket>(blockToHold, preventChange);
}
