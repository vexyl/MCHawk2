#include "../include/Server.hpp"
#include "../include/Net/Socket.hpp"
#include "../include/ServerAPI.hpp"

using namespace Net;

#pragma region HelperMacros
#define FOREACH_PLAYER(player_arg, client_arg) \
	for (auto& obj : m_players) { \
			Player::PlayerPtr player_arg = obj.second; \
			Client* client_arg = obj_player->GetClient();
#define END_FOREACH_PLAYER }
#pragma endregion

Player::PlayerPtr Server::GetPlayer(uint8_t pid)
{
	auto iter = m_players.find(pid);
	assert(iter != m_players.end());
	return iter->second;
}

void Server::SendClientMessage(Client* client, std::string message, int messageType)
{
	client->QueuePacket(ClassicProtocol::MakeMessagePacket(messageType, message));
}

void Server::SendWrappedMessage(Client* client, std::string message, int messageType)
{
	int max = 64;
	int pos = 0;
	int length = static_cast<int>(message.length());
	std::string lastColor;

	while (pos < length) {
		int diff = length - pos;
		int count = std::min(diff, max);

		if (lastColor != "" && count + 2 > max)
			count -= 2;

		SendClientMessage(client, lastColor + message.substr(pos, count), messageType);

		if (length > max) {
			for (int i = count - 1; i >= pos; --i) {
				if (message.at(i) == '&' && (i + 1) < count) {
					char color = message.at(i + 1);
					if (color != 'f') {
						lastColor = "&";
						lastColor += color;
					}
					else {
						lastColor = "";
					}
					break;
				}
			}
		}

		pos += count;
	}
}

void Server::BroadcastMessage(std::string message, int messageType)
{
	FOREACH_PLAYER(obj_player, obj_client)
		SendWrappedMessage(obj_client, message, messageType);
	END_FOREACH_PLAYER
}

Server* Server::thisPtr = nullptr;

Server::~Server()
{
	for (auto& client : m_unauthorizedClients)
		delete client;

	TCPSocket::Cleanup();
}

Server* Server::GetInstance()
{
	if (thisPtr == nullptr)
		thisPtr = new Server();
	return thisPtr;
}

void Server::Init()
{
	TCPSocket::Initialize();
	m_socket.Bind(25565);
	m_socket.Listen();

	auto classicProtocol = reinterpret_cast<ClassicProtocol*>(m_protocolHandler.GetProtocol("ClassicProtocol"));
	auto extProtocol = reinterpret_cast<ExtendedProtocol*>(m_protocolHandler.GetProtocol("ExtendedProtocol"));

	classicProtocol->onAuthenticationCallback = (
		[&](Client* client, const ClassicProtocol::AuthenticationPacket& packet)
		{
			OnAuthenticationPacket(client, packet);
		}
	);
	classicProtocol->onSetBlockCallback = (
		[&](Client* client, const ClassicProtocol::SetBlockPacket& packet)
		{
			OnSetBlockPacket(client, packet);
		}
	);
	classicProtocol->onPositionOrientationCallback = (
		[&](Client* client, const ClassicProtocol::PositionOrientationPacket& packet)
		{
			OnPositionOrientationPacket(client, packet);
		}
	);
	classicProtocol->onMessageCallback = (
		[&](Client* client, const ClassicProtocol::MessagePacket& packet)
		{
			OnMessagePacket(client, packet);
		}
	);

	extProtocol->onExtInfoCallback = (
		[&](Client* client, const ExtendedProtocol::ExtInfoPacket& packet)
		{
			//std::cout << "ExtInfo: " << packet.appName.ToString() << " | " << packet.extensionCount << std::endl;
		}
	);

	extProtocol->onExtEntryCallback = (
		[&](Client* client, const ExtendedProtocol::ExtEntryPacket& packet)
		{
			//std::cout << "ExtEntry: " << packet.extName.ToString() << " | " << packet.version << std::endl;
			Player::PlayerPtr player = GetPlayer(client->GetID());
			std::string extName = packet.extName.ToString();
			auto search = m_cpeEntries.find(extName);
			if (search == m_cpeEntries.end() || packet.version != search->second.version)
				return;

			player->AddCPEEntry(extName, packet.version);
			std::cout << player->GetName() << " CPE Ext: " << extName << " version " << packet.version << std::endl;

			if (extName == "CustomBlocks") {
				// TODO: Have init function take care of this
				client->QueuePacket(ExtendedProtocol::MakeCustomBlocksPacket(1));
			}
		}
	);

	extProtocol->onPlayerClickCallback = (
		[&](Client* client, const ExtendedProtocol::PlayerClickPacket& packet)
		{
			playerClickEvents.Trigger(client, packet);
			//std::cout << "[PlayerClick] " << std::to_string(packet.action) << ", " << std::to_string(packet.button) << "," << " | " << std::to_string(packet.targetBlockX) << ", " << std::to_string(packet.targetBlockY) << ", " << std::to_string(packet.targetBlockZ) << " | " << std::to_string(packet.targetEntityID) << std::endl;
		}
	);

	std::shared_ptr<World> world = MakeDefaultWorld();
	m_worlds[world->GetName()] = std::move(world);

	m_serverName = "MCHawk2";
	m_serverMOTD = "Welcome to a world of blocks!";

	AddCPEEntry("CustomBlocks", 1);
	AddCPEEntry("PlayerClick", 1);
	AddCPEEntry("HeldBlock", 1);

	LOG(LOGLEVEL_INFO, "Server initialized and listening on port %d", m_socket.GetPort());
}

void Server::ProcessUnauthorizedClients()
{
	auto iter = m_unauthorizedClients.begin();
	while (iter != m_unauthorizedClients.end()) {
		Net::Client* client = *iter;

		if (!client->KeepAlive()) {
			delete client;
			iter = m_unauthorizedClients.erase(iter);
			continue;
		}

		Net::Socket* socket = client->GetSocket();
		if (!socket->IsActive()) {
			LOG(LOGLEVEL_INFO, "Unauthorized client disconnected (%s)", socket->GetIPAddress().c_str());
			client->Kill();
			continue;
		}

		if (socket->Poll()) {
			if (socket->PeekFirstByte() != ClassicProtocol::kAuthentication) {
				LOG(LOGLEVEL_INFO, "Unauthorized client sent packet before authenticating (%s)", socket->GetIPAddress().c_str());
				client->Kill();
				continue;
			}

			auto result = m_protocolHandler.HandleMessage(client);
			if (result == ProtocolHandler::MessageStatus::kSuccess) {
				if (client->IsAuthorized()) {
					iter = m_unauthorizedClients.erase(iter);
					continue;
				}
				else {
					LOG(LOGLEVEL_INFO, "Client authorization failed (%s)", socket->GetIPAddress().c_str());
					client->Kill();
					continue;
				}
			}
		}

		client->ProcessPacketsInQueue();

		++iter;
	}
}

void Server::UpdatePlayers()
{
	auto iter = m_players.begin();
	while (iter != m_players.end()) {
		Player::PlayerPtr player = iter->second;
		Net::Client* client = player->GetClient();
		std::string name = iter->second->GetName();

		if (!client->KeepAlive()) {
			iter->second->GetWorld()->RemovePlayer(client->GetID());
			iter = m_players.erase(iter);
			continue;
		}

		Net::Socket* socket = client->GetSocket();
		if (!socket->IsActive()) {
			LOG(LOGLEVEL_INFO, "Player '%s' disconnected (%s)", name.c_str(), socket->GetIPAddress().c_str());
			client->Kill();
			continue;
		}

		if (socket->Poll()) {
			ProtocolHandler::MessageStatus result;
			do {
				result = m_protocolHandler.HandleMessage(client);
				if (result == ProtocolHandler::MessageStatus::kUnknownOpcode) {
					LOG(LOGLEVEL_INFO, "Player %s sent unknown packet '%d' (%s)", name.c_str(), socket->PeekFirstByte(), socket->GetIPAddress().c_str());
					client->Kill();
					continue;
				}
			} while (result == ProtocolHandler::MessageStatus::kSuccess);
		}

		client->ProcessPacketsInQueue();

		++iter;
	}
}

void Server::CheckForConnections()
{
	Net::Socket* new_socket = m_socket.Accept();
	if (new_socket != nullptr) {
		Net::Client* client = new Client(new_socket);
		m_unauthorizedClients.push_back(client);
		LOG(LOGLEVEL_INFO, "New connection (%s)", client->GetIPAddress().c_str());
	}
}

bool Server::Update()
{
	CheckForConnections();
	ProcessUnauthorizedClients();
	UpdatePlayers();

	for (auto& world : m_worlds)
		world.second->Update();

	return m_running;
}

void Server::Shutdown()
{
	m_running = false;
}

void Server::AddCPEEntry(std::string name, uint8_t version)
{
	CPEEntry entry = { name, version };
	m_cpeEntries.insert(std::make_pair(name, entry));
}

void Server::OnAuthenticationPacket(Client* client, const ClassicProtocol::AuthenticationPacket& packet)
{
	std::string name = packet.name.ToString();

	LOG(LOGLEVEL_INFO, "Player '%s' authorized with key %s", name.c_str(), packet.key.ToString().c_str());

	auto pair = m_players.emplace(client->GetID(), std::make_shared<Player>(client));

	assert(pair.second == true);

	Player::PlayerPtr player = pair.first->second;
	player->SetName(name);

	client->QueuePacket(ClassicProtocol::MakeServerIdentificationPacket(ClassicProtocol::kVersion, m_serverName, m_serverMOTD, 0));

	m_worlds["default"]->AddPlayer(player);
	client->SetAuthorized(true);

	// FIXME: TEMPORARY
	ServerAPI::SetUserType(nullptr, client, 0x64);
	m_privHandler.GivePrivilege(player->GetName(), "MapSetBlock");
	m_privHandler.GivePrivilege(player->GetName(), "chat");

	if (packet.UNK0 == 0x42) {
		LOG(LOGLEVEL_DEBUG, "Client supports CPE, sending info.");
		client->QueuePacket(ExtendedProtocol::MakeExtInfoPacket(m_serverName, m_cpeEntries.size()));
		for (auto& search : m_cpeEntries) {
			client->QueuePacket(ExtendedProtocol::MakeExtEntryPacket(Utils::MCString(search.second.name), search.second.version));
		}
	}

	authEvents.Trigger(client, packet);
}

void Server::OnSetBlockPacket(Client* client, const ClassicProtocol::SetBlockPacket& packet)
{
	setBlockEvents.Trigger(client, packet);

	if (m_blockDefaultEventHandler) {
		m_blockDefaultEventHandler = false;
		return;
	}

	Player::PlayerPtr player = GetPlayer(client->GetID());
	player->GetWorld()->OnSetBlockPacket(player, packet);
}

void Server::OnPositionOrientationPacket(Client* client, const ClassicProtocol::PositionOrientationPacket& packet)
{
	positionOrientationEvents.Trigger(client, packet);

	if (m_blockDefaultEventHandler) {
		m_blockDefaultEventHandler = false;
		return;
	}

	Player::PlayerPtr player = GetPlayer(client->GetID());
	player->GetWorld()->OnPositionOrientationPacket(player, packet);
}

void Server::OnMessagePacket(Client* client, const ClassicProtocol::MessagePacket& packet)
{
	messageEvents.Trigger(client, packet);
	ServerAPI::BroadcastMessage(nullptr, client, packet.message.ToString());
}

std::shared_ptr<World> MakeDefaultWorld()
{
	std::unique_ptr<Map> map;
	map = MapGen::GenerateFlatMap(256, 64, 256);

	std::shared_ptr<World> world = std::make_shared<World>("default");
	world->SetSpawnPosition(Utils::Vector(256 / 2, 64 / 2, 256 / 2));
	world->SetMap(std::move(map));

	return world;
}