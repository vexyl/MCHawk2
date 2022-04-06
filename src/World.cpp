#include "../include/World.hpp"
#include "../include/Server.hpp"
#include "../include/Net/ClassicProtocol.hpp"
#include "../include/ServerAPI.hpp"
#include "../include/Utils/Vector.hpp"
#include "../include/Utils/Utils.hpp"

#include <memory>

using namespace Net;

#pragma region HelperMacros
#define GET_PLAYER_VARIABLES(pid_arg, player_arg, client_arg) \
	Player::PlayerPtr player_arg = Server::GetInstance()->GetPlayer(pid_arg); \
	Client* client_arg = player_arg->GetClient();
#define FOREACH_PLAYER(player_arg, client_arg) \
	for (Player::PlayerPtr player_arg : m_players)  { \
			Client* client_arg = player_arg->GetClient();
#define END_FOREACH_PLAYER }
#pragma endregion

int8_t World::pid = 0;

void World::SetWeatherType(WeatherType type)
{
	if (m_weatherType != type) {
		m_weatherType = type;
		for (Player::PlayerPtr player : m_players)
			SendWeatherType(player);
	}
}

void World::AddPlayer(Player::PlayerPtr player)
{
	Net::Client* client = player->GetClient();
	int8_t pid = World::pid++;

	player->SetPID(pid);

	std::string name = player->GetName();

	SendLevel(client);

	player->SetPosition(m_spawnPosition);

	Utils::Vector convertedPosition = Utils::ConvertBlockToPlayer(m_spawnPosition);

	// Server doesn't know when last CPE entry is received
	// so if client has CPE at all, assume ExtPlayerList v2 is supported
	// otherwise, send normal SpawnPlayer
	if (player->CPEEnabled()) {
		// Spawn player
		client->QueuePacket(ExtendedProtocol::MakeExtAddEntity2Packet(
			-1, name, name,
			static_cast<int16_t>(convertedPosition.x),
			static_cast<int16_t>(convertedPosition.y),
			static_cast<int16_t>(convertedPosition.z),
			0, 0
		));

		client->QueuePacket(ExtendedProtocol::MakeExtAddPlayerNamePacket(-1, name, name, Utils::MCString(), 0));
	} else {
		// Spawn player
		client->QueuePacket(ClassicProtocol::MakeSpawnPlayerPacket(
			-1, name,
			static_cast<int16_t>(convertedPosition.x),
			static_cast<int16_t>(convertedPosition.y),
			static_cast<int16_t>(convertedPosition.z),
			0, 0
		));
	}

	std::shared_ptr<Net::Packet> spawnPacket = nullptr, spawnPacketCPE = nullptr;

	FOREACH_PLAYER(otherPlayer, otherClient)
		if (otherPlayer->CPEEnabled()) {
			if (spawnPacketCPE == nullptr) {
				spawnPacketCPE = ExtendedProtocol::MakeExtAddEntity2Packet(
					pid, name, name,
					static_cast<int16_t>(convertedPosition.x),
					static_cast<int16_t>(convertedPosition.y),
					static_cast<int16_t>(convertedPosition.z),
					0, 0
				);
			}

			// Send player to other players
			otherClient->QueuePacket(spawnPacketCPE);
			otherClient->QueuePacket(ExtendedProtocol::MakeExtAddPlayerNamePacket(pid, name, name, Utils::MCString(), 0));
		} else {
			spawnPacket = ClassicProtocol::MakeSpawnPlayerPacket(
				pid, name,
				static_cast<int16_t>(convertedPosition.x),
				static_cast<int16_t>(convertedPosition.y),
				static_cast<int16_t>(convertedPosition.z),
				0, 0
			);

			// Send player to other players
			otherClient->QueuePacket(spawnPacket);
		}

		// Send other players to player
		int8_t spawnPlayerPid = otherPlayer->GetPID();
		Utils::Vector pos = otherPlayer->GetPosition();
		uint8_t yaw = otherPlayer->GetYaw();
		uint8_t pitch = otherPlayer->GetPitch();

		if (player->CPEEnabled()) {
			client->QueuePacket(ExtendedProtocol::MakeExtAddEntity2Packet(
				spawnPlayerPid,
				otherPlayer->GetName(), otherPlayer->GetName(),
				static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y), static_cast<int16_t>(pos.z), yaw, pitch)
			);

			client->QueuePacket(ExtendedProtocol::MakeExtAddPlayerNamePacket(spawnPlayerPid, otherPlayer->GetName(), otherPlayer->GetName(), Utils::MCString(), 0));
		} else {
			client->QueuePacket(ClassicProtocol::MakeSpawnPlayerPacket(
				spawnPlayerPid,
				otherPlayer->GetName(),
				static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y), static_cast<int16_t>(pos.z), yaw, pitch)
			);
		}
	END_FOREACH_PLAYER

	player->SetWorld(this);
	m_players.push_back(player);

	Server::GetInstance()->GetPluginHandler().TriggerJoinEvent(player, player->GetWorld());

	SendBlockPermissions(player);
}

void World::RemovePlayer(int8_t pid)
{
	auto iter = std::find_if(m_players.begin(), m_players.end(),
		[&](const Player::PlayerPtr player) { return player->GetPID() == pid; });

	assert(iter != m_players.end());

	(*iter)->SetWorld(nullptr);
	m_players.erase(iter);

	FOREACH_PLAYER(player, client)
		client->QueuePacket(ClassicProtocol::MakeDespawnPacket(pid));
		if (player->CPEEnabled())
			client->QueuePacket(ExtendedProtocol::MakeExtRemovePlayerNamePacket(pid));
	END_FOREACH_PLAYER
}

void World::AddBlockDef(BlockDef def)
{
	m_blockDefinitions.push_back(def);
}

void World::Update()
{

}

void World::SendLevel(Client* client)
{
	auto levelInitializePacket = ClassicProtocol::MakeLevelInitializePacket();
	client->QueuePacket(levelInitializePacket);

	uint8_t* compBuffer = nullptr;
	size_t compSize;

	Utils::CompressBuffer(m_map->GetReadOnlyBufferPtr(), m_map->GetBufferSize(), &compBuffer, &compSize);

	LOG(LOGLEVEL_DEBUG, "Compressed map size: %d bytes", compSize);

	size_t bytes = 0;
	while (bytes < compSize) {
		size_t remainingBytes = compSize - bytes;
		size_t count = (remainingBytes >= 1024) ? 1024 : (remainingBytes);

		auto chunkPacket = std::make_shared<ClassicProtocol::LevelDataChunkPacket>();

		chunkPacket->chunkLength = static_cast<uint16_t>(count);

		std::memcpy(chunkPacket->chunkData, &compBuffer[bytes], count);

		// Padding; must send exactly 1024 bytes per chunk
		if (count < 1024) {
			size_t paddingSize = 1024 - count;
			std::memset(&chunkPacket->chunkData[count], 0x00, paddingSize);
		}

		bytes += count;

		// std::cout << bytes << "/" << compSize << " bytes" << std::endl;

		chunkPacket->percent = static_cast<uint8_t>((((float)bytes / (float)compSize) * 100.0f));

		client->QueuePacket(chunkPacket);
		client->ProcessPacketsInQueue();
	}

	delete compBuffer;

	auto levelFinalizePacket = ClassicProtocol::MakeLevelFinalizePacket(m_map->GetXSize(), m_map->GetYSize(), m_map->GetZSize());

	client->QueuePacket(levelFinalizePacket);
}

void World::SendWeatherType(Player::PlayerPtr player)
{
	uint8_t version = Server::GetInstance()->GetCPEEntryVersion("EnvWeatherType");
	if (player->HasCPEEntry("EnvWeatherType", version))
		player->GetClient()->QueuePacket(Net::ExtendedProtocol::MakeEnvSetWeatherTypePacket(static_cast<uint8_t>(m_weatherType)));
}

void World::SendBlockDefinitions(Player::PlayerPtr player)
{
	uint8_t version = player->GetCPEEntryVersion("BlockDefinitions");
	if (version == 0)
		return;

	std::shared_ptr<Net::Packet> packet;

	for (auto& def : m_blockDefinitions) {
		if (!def.useBlockDefinitionsExt || version == 1) {
			packet = Net::ExtendedProtocol::MakeDefineBlockPacket(
				def.blockID,
				def.name,
				def.solidity,
				def.movementSpeed,
				def.topTextureID, def.sideTextureID, def.bottomTextureID,
				def.transmitLight,
				def.walkSound,
				def.fullBright,
				def.shape,
				def.blockDraw,
				def.fogDensity,
				def.fogR, def.fogG, def.fogB
			);
		} else if (def.useBlockDefinitionsExt && version == 2) {
			packet = Net::ExtendedProtocol::MakeDefineBlockExtPacket(
				def.blockID,
				def.name,
				def.solidity,
				def.movementSpeed,
				def.topTextureID, def.leftTextureID, def.rightTextureID,
				def.frontTextureID, def.backTextureID, def.bottomTextureID,
				def.transmitLight,
				def.walkSound,
				def.fullBright,
				def.minX, def.minY, def.minZ,
				def.maxX, def.maxY, def.maxZ,
				def.blockDraw,
				def.fogDensity,
				def.fogR, def.fogG, def.fogB
			);
		}

		player->GetClient()->QueuePacket(packet);
	}
}

void World::SendBlockPermissions(Player::PlayerPtr player)
{
	if (player->HasCPEEntry("BlockPermissions", 1)) {
		Net::Client* client = player->GetClient();
		auto result = Server::GetInstance()->GetPrivilegeHandler().HasPrivilege(player->GetName(), "build");
		if (result.error) {
			for (auto& def : m_blockDefinitions) {
				client->QueuePacket(ExtendedProtocol::MakeSetBlockPermissionPacket(def.blockID, 0, 0));
			}
			for (auto iter = ClassicProtocol::blockTypes.begin(); iter != ClassicProtocol::blockTypes.end(); ++iter) {
				client->QueuePacket(ExtendedProtocol::MakeSetBlockPermissionPacket(iter->first, 0, 0));
			}
			for (auto iter = ExtendedProtocol::blockTypes.begin(); iter != ExtendedProtocol::blockTypes.end(); ++iter) {
				client->QueuePacket(ExtendedProtocol::MakeSetBlockPermissionPacket(iter->first, 0, 0));
			}
		}
	}
}

void World::OnSetBlockPacket(Player::PlayerPtr player, const ClassicProtocol::SetBlockPacket& packet)
{
	Net::Client* client = player->GetClient();

	uint8_t blockType = packet.type;
	if (packet.mode == 0)
		blockType = 0x00;

	auto p = ClassicProtocol::MakeSetBlock2Packet(packet.x, packet.y, packet.z, blockType);

	Net::ProtocolHandler& protocolHandler = Server::GetInstance()->GetProtocolHandler();
	std::string blockName = protocolHandler.GetBlockNameByType(packet.type);

	std::cout
		<< "SetBlock type=" << static_cast<unsigned>(packet.type) << " (" << blockName << ")"
		<< ", mode=" << static_cast<unsigned>(packet.mode)
		<< " @ ("
		<< static_cast<short>(packet.x) << ", "
		<< static_cast<short>(packet.y) << ", "
		<< static_cast<short>(packet.z) << ")"
		<< std::endl;

	Utils::Vector pos(packet.x, packet.y, packet.z);

	// TODO: Kick player
	if (!protocolHandler.IsValidBlock(packet.type)) {
		uint8_t actualType = m_map->PeekBlock(pos);
		client->QueuePacket(ClassicProtocol::MakeSetBlock2Packet(packet.x, packet.y, packet.z, actualType));
		return;
	}

	if (!ServerAPI::MapSetBlock(client, m_map.get(), pos, blockType))
		return;

	uint8_t pid = player->GetPID();
	FOREACH_PLAYER(obj_player, obj_client)
		if (obj_player->GetPID() != pid)
			obj_client->QueuePacket(p);
	END_FOREACH_PLAYER
}

void World::OnPositionOrientationPacket(Player::PlayerPtr player, const ClassicProtocol::PositionOrientationPacket& packet)
{
	int8_t srcPid = player->GetPID();
	Utils::Vector position = player->GetPosition();
	Utils::Vector newPosition(static_cast<float>(packet.x), static_cast<float>(packet.y), static_cast<float>(packet.z));

	bool doOrientationUpdate = false, doPositionUpdate = false;
	uint8_t yaw = player->GetYaw(), pitch = player->GetPitch();

	if (player->GetPitch() != packet.pitch || player->GetYaw() != packet.yaw) {
		yaw = packet.yaw;
		pitch = packet.pitch;
		player->SetOrientation(packet.pitch, packet.yaw);
		doOrientationUpdate = true;
	}

	if (newPosition != position) {
		player->SetPosition(newPosition);
		doPositionUpdate = true;
	}

	if (doPositionUpdate == true) {
		auto positionOrientationPacket = ClassicProtocol::MakePositionOrientationPacket(srcPid, packet.x, packet.y, packet.z, yaw, pitch);

		FOREACH_PLAYER(obj_player, obj_client)
			int8_t destPid = obj_player->GetPID();
			if (srcPid != destPid)
				obj_client->QueuePacket(positionOrientationPacket);
		END_FOREACH_PLAYER

			/*std::cout
				<< "PositionOrientation: " << player->GetName()
				<< "(pid=" << static_cast<signed>(srcPid) << ")"
				<< " @ ("
				<< static_cast<short>(packet.x) << ", "
				<< static_cast<short>(packet.y) << ", "
				<< static_cast<short>(packet.z) << ")"
				<< ", yaw=" << static_cast<unsigned>(packet.yaw)
				<< ", pitch=" << static_cast<unsigned>(packet.pitch)
				<< std::endl;*/
	} else if (doOrientationUpdate == true) {
		auto orientationPacket = ClassicProtocol::MakeOrientationPacket(srcPid, packet.yaw, packet.pitch);

		FOREACH_PLAYER(obj_player, obj_client)
			int8_t destPid = obj_player->GetPID();
			if (srcPid != destPid)
				obj_client->QueuePacket(orientationPacket);
		END_FOREACH_PLAYER

		/*std::cout
			<< "OrientationUpdate: " << player->GetName()
			<< "(pid=" << static_cast<signed>(srcPid) << ")"
			<< " yaw=" << static_cast<unsigned>(packet.yaw)
			<< ", pitch=" << static_cast<unsigned>(packet.pitch)
			<< std::endl;*/
	}

	// CPE Held Block uses pid field of this packet for block type
	uint8_t version = Server::GetInstance()->GetCPEEntryVersion("HeldBlock");
	if (player->HasCPEEntry("HeldBlock", version))
		player->heldBlock = packet.pid;
}

// FIXME
void World::NewBlockDef(
	uint8_t blockID,
	std::string name,
	uint8_t solidity,
	uint8_t movementSpeed,
	uint8_t topTextureID, uint8_t sideTextureID, uint8_t bottomTextureID,
	uint8_t transmitLight,
	uint8_t walkSound,
	uint8_t fullBright,
	uint8_t shape,
	uint8_t blockDraw,
	uint8_t fogDensity,
	uint8_t fogR, uint8_t fogG, uint8_t fogB
)
{
	BlockDef def;
	def.blockID = blockID;
	def.name = name;
	def.solidity = solidity;
	def.movementSpeed = movementSpeed;
	def.topTextureID = topTextureID;
	def.sideTextureID = sideTextureID;
	def.bottomTextureID = bottomTextureID;
	def.transmitLight = transmitLight;
	def.walkSound = walkSound;
	def.fullBright = fullBright;
	def.shape = shape;
	def.blockDraw = blockDraw;
	def.fogDensity = fogDensity;
	def.fogR = fogR;
	def.fogG = fogG;
	def.fogB = fogB;

	AddBlockDef(def);
}