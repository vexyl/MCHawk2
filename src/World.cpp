#include "../include/World.hpp"
#include "../include/Server.hpp"
#include "../include/Net/ClassicProtocol.hpp"
#include "../include/ServerAPI.hpp"
#include "../include/Position.hpp"

using namespace Net;

#pragma region HelperMacros
#define GET_PLAYER_VARIABLES(pid_arg, player_arg, client_arg) \
	Player& player_arg = Server::GetInstance()->GetPlayer(pid_arg); \
	Client* client_arg = player_arg.GetClient();
#define FOREACH_PLAYER(player_arg, client_arg) \
	for (auto foreach_player_pid : m_playerPids)  { \
		GET_PLAYER_VARIABLES(foreach_player_pid, player_arg, client_arg)
#define END_FOREACH_PLAYER }
#pragma endregion

void World::AddPlayer(int8_t pid)
{
	GET_PLAYER_VARIABLES(pid, player, client);

	std::string name = player.GetName();

	SendLevel(client);

	// Spawn player
	client->QueuePacket(ClassicProtocol::MakeSpawnPlayerPacket(-1, name, m_spawnPosition.x * 32 + 51, m_spawnPosition.y * 32 + 51, m_spawnPosition.z * 32 + 51, 0, 0));

	auto spawnPacket = ClassicProtocol::MakeSpawnPlayerPacket(pid, name, m_spawnPosition.x * 32 + 51, m_spawnPosition.y * 32 + 51, m_spawnPosition.z * 32 + 51, 0, 0);
	FOREACH_PLAYER(otherPlayer, otherClient)
		// Send player to other players
		otherClient->QueuePacket(spawnPacket);

		// Send other players to player
		int8_t spawnPlayerPid = foreach_player_pid; // foreach_player_pid hidden in FOREACH_PLAYER
		Position pos = otherPlayer.GetPosition();
		uint8_t yaw = otherPlayer.GetYaw();
		uint8_t pitch = otherPlayer.GetPitch();

		client->QueuePacket(ClassicProtocol::MakeSpawnPlayerPacket(spawnPlayerPid, otherPlayer.GetName(), pos.x, pos.y, pos.z, yaw, pitch));
	END_FOREACH_PLAYER

	player.SetWorld(this);
	m_playerPids.push_back(pid);

	// TEMPORARY
	ServerAPI::SetUserType(nullptr, client, 0x64);

	ServerAPI::SendClientMessage(nullptr, client, "&bWelcome to the world, " + name + "!");
}

void World::RemovePlayer(int8_t pid)
{
	m_playerPids.erase(std::remove_if(m_playerPids.begin(), m_playerPids.end(),
		[&](const int8_t checkPid) { return checkPid == pid; }),
		m_playerPids.end());

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

void World::OnSetBlockPacket(Client* client, const ClassicProtocol::SetBlockPacket& packet)
{
	auto p = ClassicProtocol::MakeSetBlock2Packet(packet.x, packet.y, packet.z, packet.type);

	if (packet.mode == 0)
		p->type = 0x00;

	Position pos(p->x, p->y, p->z);

	if (!ClassicProtocol::IsValidBlock(packet.type)) {
		uint8_t actualType = m_map->PeekBlock(pos);
		client->QueuePacket(ClassicProtocol::MakeSetBlock2Packet(packet.x, packet.y, packet.z, actualType));
		return;
	}

	ServerAPI::MapSetBlock(client, m_map.get(), pos, p->type);
	
	uint8_t pid = client->GetPID();
	FOREACH_PLAYER(obj_player, obj_client)
		if (obj_client->GetPID() != pid)
			obj_client->QueuePacket(p);
	END_FOREACH_PLAYER
}

void World::OnPositionOrientationPacket(Client* client, const ClassicProtocol::PositionOrientationPacket& packet)
{
	int8_t srcPid = client->GetPID();
	Player& player = Server::GetInstance()->GetPlayer(srcPid);
	Position position = player.GetPosition();

	bool doOrientationUpdate = false, doPositionUpdate = false;

	if (player.GetPitch() != packet.pitch || player.GetYaw() != packet.yaw) {
		player.SetOrientation(packet.pitch, packet.yaw);
		doOrientationUpdate = true;
	}

	if (position.x != packet.x || position.y != packet.y || position.z != packet.z) {
		player.SetPosition(Position(packet.x, packet.y, packet.z));
		doPositionUpdate = true;
	}

	if (doPositionUpdate == true) {
		auto positionOrientationPacket = ClassicProtocol::MakePositionOrientationPacket(srcPid, packet.x, packet.y, packet.z, packet.yaw, packet.pitch);

		FOREACH_PLAYER(obj_player, obj_client)
			int8_t destPid = obj_client->GetPID();
		if (srcPid != destPid)
			obj_client->QueuePacket(positionOrientationPacket);
		END_FOREACH_PLAYER

		/*std::cout
			<< "PositionOrientation: " << player.GetName()
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
			int8_t destPid = obj_client->GetPID();
		if (srcPid != destPid)
			obj_client->QueuePacket(orientationPacket);
		END_FOREACH_PLAYER

		/*std::cout
			<< "OrientationUpdate: " << player.GetName()
			<< "(pid=" << static_cast<signed>(srcPid) << ")"
			<< " yaw=" << static_cast<unsigned>(packet.yaw)
			<< ", pitch=" << static_cast<unsigned>(packet.pitch)
			<< std::endl;*/
	}
}
