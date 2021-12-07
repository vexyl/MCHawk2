#include "../include/World.hpp"
#include "../include/Server.hpp"
#include "../include/Net/ClassicProtocol.hpp"
#include "../include/ServerAPI.hpp"
#include "../include/Utils/Vector.hpp"
#include "../include/Utils/Utils.hpp"

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

void World::AddPlayer(Player::PlayerPtr player)
{
	Net::Client* client = player->GetClient();
	int8_t pid = World::pid++;

	player->SetID(pid);

	std::string name = player->GetName();

	SendLevel(client);

	player->SetPosition(m_spawnPosition);

	Utils::Vector convertedPosition = Utils::ConvertBlockToPlayer(m_spawnPosition);

	// Spawn player
	client->QueuePacket(ClassicProtocol::MakeSpawnPlayerPacket(
		-1, name,
		static_cast<int16_t>(convertedPosition.x),
		static_cast<int16_t>(convertedPosition.y),
		static_cast<int16_t>(convertedPosition.z),
		0, 0
	));

	auto spawnPacket = ClassicProtocol::MakeSpawnPlayerPacket(
		pid, name,
		static_cast<int16_t>(convertedPosition.x),
		static_cast<int16_t>(convertedPosition.y),
		static_cast<int16_t>(convertedPosition.z),
		0, 0
	);

	FOREACH_PLAYER(otherPlayer, otherClient)
		// Send player to other players
		otherClient->QueuePacket(spawnPacket);

	// Send other players to player
	int8_t spawnPlayerPid = otherPlayer->GetID();
	Utils::Vector pos = otherPlayer->GetPosition();
	uint8_t yaw = otherPlayer->GetYaw();
	uint8_t pitch = otherPlayer->GetPitch();

	client->QueuePacket(ClassicProtocol::MakeSpawnPlayerPacket(
		spawnPlayerPid,
		otherPlayer->GetName(),
		static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y), static_cast<int16_t>(pos.z), yaw, pitch)
	);
	END_FOREACH_PLAYER

		player->SetWorld(this);
	m_players.push_back(player);
}

void World::RemovePlayer(int8_t pid)
{
	auto iter = std::find_if(m_players.begin(), m_players.end(),
		[&](const Player::PlayerPtr player) { return player->GetClient()->GetID() == pid; });

	(*iter)->SetWorld(nullptr);
	m_players.erase(iter);

	FOREACH_PLAYER(player, client)
		client->QueuePacket(ClassicProtocol::MakeDespawnPacket(pid));
	END_FOREACH_PLAYER
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

	m_map->CompressBuffer(&compBuffer, &compSize);

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

void World::OnSetBlockPacket(Player::PlayerPtr player, const ClassicProtocol::SetBlockPacket& packet)
{
	Net::Client* client = player->GetClient();
	auto p = ClassicProtocol::MakeSetBlock2Packet(packet.x, packet.y, packet.z, packet.type);

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

	if (packet.mode == 0)
		p->type = 0x00;

	Utils::Vector pos(p->x, p->y, p->z);

	// TODO: Kick player
	if (!protocolHandler.IsValidBlock(packet.type)) {
		uint8_t actualType = m_map->PeekBlock(pos);
		client->QueuePacket(ClassicProtocol::MakeSetBlock2Packet(packet.x, packet.y, packet.z, actualType));
		return;
	}

	if (!ServerAPI::MapSetBlock(client, m_map.get(), pos, p->type))
		return;

	uint8_t pid = player->GetID();
	FOREACH_PLAYER(obj_player, obj_client)
		if (obj_player->GetID() != pid)
			obj_client->QueuePacket(p);
	END_FOREACH_PLAYER
}

void World::OnPositionOrientationPacket(Player::PlayerPtr player, const ClassicProtocol::PositionOrientationPacket& packet)
{
	int8_t srcPid = player->GetID();
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
			int8_t destPid = obj_player->GetID();
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
	}
	else if (doOrientationUpdate == true) {
		auto orientationPacket = ClassicProtocol::MakeOrientationPacket(srcPid, packet.yaw, packet.pitch);

		FOREACH_PLAYER(obj_player, obj_client)
			int8_t destPid = obj_player->GetID();
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
	if (player->HasCPEExtension("HeldBlock", 1))
		player->heldBlock = packet.pid;
}
