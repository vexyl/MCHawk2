#include "../include/Server.hpp"
#include "../include/Net/Socket.hpp"
#include "../include/Utils/Utils.hpp"

using namespace Net;

Player::PlayerPtr Server::GetPlayer(uint8_t pid)
{
	auto iter = m_players.find(pid);
	assert(iter != m_players.end());
	return iter->second;
}

void Server::SendClientMessage(std::shared_ptr<Client> client, std::string message, int messageType)
{
	client->QueuePacket(ClassicProtocol::MakeMessagePacket(messageType, message));
}

void Server::SendWrappedMessage(std::shared_ptr<Client> client, std::string message, int messageType)
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
					} else {
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
	for (auto& player : m_players)
		SendWrappedMessage(player.second->GetClient(), message, messageType);
}

Server* Server::thisPtr = nullptr;

Server::~Server()
{
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

	auto classicProtocol = static_cast<ClassicProtocol*>(m_protocolHandler.GetProtocol("ClassicProtocol"));
	auto extProtocol = static_cast<ExtendedProtocol*>(m_protocolHandler.GetProtocol("ExtendedProtocol"));

	classicProtocol->onAuthenticationCallback = (
		[this](std::shared_ptr<Client> client, const ClassicProtocol::AuthenticationPacket& packet)
		{
			OnAuthenticationPacket(client, packet);
		}
	);

	classicProtocol->onSetBlockCallback = (
		[this](std::shared_ptr<Client> client, const ClassicProtocol::SetBlockPacket& packet)
		{
			OnSetBlockPacket(client, packet);
		}
	);

	classicProtocol->onPositionOrientationCallback = (
		[this](std::shared_ptr<Client> client, const ClassicProtocol::PositionOrientationPacket& packet)
		{
			OnPositionOrientationPacket(client, packet);
		}
	);

	classicProtocol->onMessageCallback = (
		[this](std::shared_ptr<Client> client, const ClassicProtocol::MessagePacket& packet)
		{
			OnMessagePacket(client, packet);
		}
	);

	extProtocol->onExtInfoCallback = (
		[this](std::shared_ptr<Client> client, const ExtendedProtocol::ExtInfoPacket& packet)
		{
			//std::cout << "ExtInfo: " << packet.appName.ToString() << " | " << packet.extensionCount << std::endl;
		}
	);

	extProtocol->onExtEntryCallback = (
		[this](std::shared_ptr<Client> client, const ExtendedProtocol::ExtEntryPacket& packet)
		{
			//std::cout << "ExtEntry: " << packet.extName.ToString() << " | " << packet.version << std::endl;
			Player::PlayerPtr player = GetPlayer(client->GetSID());
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

			if (extName == "EnvWeatherType") {
				// TODO: Have init function take care of this
				client->QueuePacket(ExtendedProtocol::MakeEnvSetWeatherTypePacket(static_cast<uint8_t>(player->GetWorld()->GetWeatherType())));
			}

			if (extName == "BlockDefinitions") {
				// TODO: Have init function take care of this
				player->GetWorld()->SendBlockDefinitions(player);
			}

			if (extName == "BlockPermissions") {
				// TODO: Have init function take care of this
				player->GetWorld()->SendBlockPermissions(player);
			}
		}
	);

	extProtocol->onPlayerClickedCallback = (
		[this](std::shared_ptr<Client> client, const ExtendedProtocol::PlayerClickedPacket& packet)
		{
			std::cout << "[PlayerClick] " << std::to_string(packet.action) << ", " << std::to_string(packet.button) << "," << " | " << std::to_string(packet.targetBlockX) << ", " << std::to_string(packet.targetBlockY) << ", " << std::to_string(packet.targetBlockZ) << " | " << std::to_string(packet.targetEntityID) << std::endl;
		}
	);

	extProtocol->onTwoWayPingCallback = (
		[this](std::shared_ptr<Client> client, const ExtendedProtocol::TwoWayPingPacket& packet)
		{
			// TODO
		}
	);

	m_worlds["default"] = MakeDefaultWorld();

	m_serverName = "MCHawk2";
	m_serverMOTD = "Welcome to a world of blocks!";

	AddCPEEntry("CustomBlocks", 1);
	AddCPEEntry("HeldBlock", 1);
	AddCPEEntry("InventoryOrder", 1);
	AddCPEEntry("SelectionCuboid", 1);
	AddCPEEntry("EnvWeatherType", 1);
	AddCPEEntry("PlayerClick", 1);
	AddCPEEntry("BlockDefinitions", 1);
	AddCPEEntry("BlockDefinitions", 2);
	AddCPEEntry("TwoWayPing", 1);
	AddCPEEntry("MessageTypes", 1);
	AddCPEEntry("BlockPermissions", 1);
	AddCPEEntry("SetHotbar", 1);
	AddCPEEntry("ExtPlayerList", 2);
	AddCPEEntry("ChangeModel", 1);

	LOG(LOGLEVEL_INFO, "Server initialized and listening on port %d", m_socket.GetPort());
}

void Server::ProcessUnauthorizedClients()
{
	auto iter = m_unauthorizedClients.begin();
	while (iter != m_unauthorizedClients.end()) {
		std::shared_ptr<Client> client = *iter;

		if (!client->KeepAlive()) {
			iter = m_unauthorizedClients.erase(iter);
			continue;
		}

		if (!client->IsSocketActive()) {
			LOG(LOGLEVEL_INFO, "Unauthorized client disconnected (%s)", client->GetIPAddress().c_str());
			client->Kill();
			continue;
		}

		if (client->PollSocket()) {
			if (client->GetCurrentOpcode() != ClassicProtocol::kAuthentication) {
				LOG(LOGLEVEL_INFO, "Unauthorized client sent packet before authenticating (%s)", client->GetIPAddress().c_str());
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
					LOG(LOGLEVEL_INFO, "Client authorization failed (%s)", client->GetIPAddress().c_str());
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
		std::shared_ptr<Client> client = player->GetClient();
		std::string name = iter->second->GetName();

		if (!client->IsSocketActive())
			client->Kill();

		if (!client->KeepAlive()) {
			iter->second->GetWorld()->RemovePlayer(player->GetPID());
			iter = m_players.erase(iter);
			BroadcastMessage("&e" + name + " disconnected");
			LOG(LOGLEVEL_INFO, "Player '%s' disconnected (%s)", name.c_str(), client->GetIPAddress().c_str());
			continue;
		}

		if (client->PollSocket()) {
			ProtocolHandler::MessageStatus result;
			do {
				result = m_protocolHandler.HandleMessage(client);
				if (result == ProtocolHandler::MessageStatus::kUnknownOpcode) {
					LOG(LOGLEVEL_INFO, "Player %s sent unknown packet '%d' (%s)", name.c_str(), client->GetCurrentOpcode(), client->GetIPAddress().c_str());
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
	std::unique_ptr<Net::Socket> new_socket = m_socket.Accept();
	if (new_socket != nullptr) {
		std::shared_ptr<Client> client = std::make_shared<Client>(new_socket);
		LOG(LOGLEVEL_INFO, "New connection (%s)", client->GetIPAddress().c_str());
		m_unauthorizedClients.push_back(std::move(client));
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

void Server::OnAuthenticationPacket(std::shared_ptr<Client> client, const ClassicProtocol::AuthenticationPacket& packet)
{
	std::string name = packet.name.ToString();

	LOG(LOGLEVEL_INFO, "Player '%s' authorized with key %s", name.c_str(), packet.key.ToString().c_str());
	BroadcastMessage("&e" + name + " connected");

	auto pair = m_players.emplace(client->GetSID(), std::make_shared<Player>(client));

	assert(pair.second == true);

	Player::PlayerPtr player = pair.first->second;
	player->SetName(name);

	client->QueuePacket(ClassicProtocol::MakeServerIdentificationPacket(ClassicProtocol::kVersion, m_serverName, m_serverMOTD, 0));
	client->SetAuthorized(true);

	if (packet.UNK0 == 0x42) {
		LOG(LOGLEVEL_DEBUG, "Client supports CPE, sending info.");
		player->SetCPEEnabled(true);
		client->QueuePacket(ExtendedProtocol::MakeExtInfoPacket(m_serverName, GetExtensionCount()));
		for (auto& search : m_cpeEntries) {
			client->QueuePacket(ExtendedProtocol::MakeExtEntryPacket(Utils::MCString(search.second.name), search.second.version));
		}
	}

	m_worlds["default"]->AddPlayer(player);
}

void Server::OnSetBlockPacket(std::shared_ptr<Client> client, const ClassicProtocol::SetBlockPacket& packet)
{
	Player::PlayerPtr player = GetPlayer(client->GetSID());
	player->GetWorld()->OnSetBlockPacket(player, packet);
}

void Server::OnPositionOrientationPacket(std::shared_ptr<Client> client, const ClassicProtocol::PositionOrientationPacket& packet)
{
	Player::PlayerPtr player = GetPlayer(client->GetSID());
	player->GetWorld()->OnPositionOrientationPacket(player, packet);
}

void Server::OnMessagePacket(std::shared_ptr<Client> client, const ClassicProtocol::MessagePacket& packet)
{
	std::string playerName = GetPlayer(client->GetSID())->GetName();
	std::string message = playerName + ": " + packet.message.ToString();
	LOG(LOGLEVEL_NORMAL, message.c_str());
	BroadcastMessage(message);
}

std::shared_ptr<World> MakeDefaultWorld()
{
	std::shared_ptr<Map> map;
	map = MapGen::GenerateFlatMap(256, 64, 256);

	std::shared_ptr<World> world = std::make_shared<World>("default");
	world->SetSpawnPosition(Utils::Vector(256 / 2, 64 / 2, 256 / 2));
	world->SetMap(std::move(map));

	return world;
}
