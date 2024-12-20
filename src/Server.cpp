#include "Server.h"

#include "Net/Socket/TCPSocket.h"
#include "Utils/Utils.h"

using namespace Net;

int64_t Linker::NextId = 0;

Server::Server()
{
	m_logger = std::make_shared<Utils::Logger>("log.txt");
}

Server::~Server()
{
	TCPSocket::Cleanup();
}

std::shared_ptr<World> Server::GetWorldByName(std::string name)
{
	std::shared_ptr<World> world;
	auto iter = m_worlds.find(name);
	if (iter != m_worlds.end())
		world = iter->second;
	return world;
}

uint8_t Server::GetCPEEntryVersion(std::string name) const
{
	uint8_t version = 0;
	auto search = m_cpeEntries.find(name);
	if (search != m_cpeEntries.end()) {
		return search->second.version;
	}

	return version;
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

	while (pos < length) {
		int diff = length - pos;
		int count = std::min(diff, max);

		SendClientMessage(client, message.substr(pos, count), messageType);

		pos += count;
	}
}

void Server::BroadcastMessage(std::string message, int messageType)
{
	auto players = m_linker.GetAll<Player>();
	for (auto& player : players)
		SendWrappedMessage(player->GetClient(), message, messageType);
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
			Player::Ptr player = m_linker.Get<Player>(client->GetId());
			std::string extName = packet.extName.ToString();
			auto search = m_cpeEntries.find(extName);
			if (search == m_cpeEntries.end() || packet.version != search->second.version)
				return;

			player->AddCPEEntry(extName, packet.version);
			LOG(LOGLEVEL_DEBUG, "%s CPE Ext: %s version %d", player->GetName().c_str(), extName.c_str(), packet.version);

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

	m_serverName = "MCHawk2";
	m_serverMOTD = "Welcome to a world of blocks!";

	if (m_worlds.find("default") == m_worlds.end())
		m_worlds["default"] = MakeDefaultWorld();

	LOG(LOGLEVEL_INFO, "Server initialized and listening on port %d", m_socket.GetPort());
}

std::shared_ptr<World> Server::MakeDefaultWorld()
{
	std::shared_ptr<Map> map;
	map = MapGen::GenerateFlatMap(256, 64, 256);

	LOG(LOGLEVEL_DEBUG, "Created default map: %dx%dx%d", map->GetXSize(), map->GetYSize(), map->GetZSize());

	std::shared_ptr<World> world = std::make_shared<World>(*this, m_logger, "default");
	world->SetSpawnPosition(Utils::Vector(256 / 2, 64 / 2, 256 / 2));
	world->SetMap(std::move(map));

	return world;
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
	auto players = m_linker.GetAll<Player>();
	auto iter = players.begin();
	while (iter != players.end()) {
		Player::Ptr player = *iter;
		std::shared_ptr<Client> client = player->GetClient();
		std::string name = player->GetName();

		if (!client->IsSocketActive())
			client->Kill();

		if (!client->KeepAlive()) {
			player->GetWorld()->RemovePlayer(player->GetPID());
			m_linker.Remove<Player>(client->GetId());
			iter = players.erase(iter);
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
	std::unique_ptr<Net::TCPSocket> new_socket = m_socket.Accept();
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
	LOG(LOGLEVEL_DEBUG, "Shutting down...");
	auto players = m_linker.GetAll<Player>();
	for (auto& player : players) {
		auto client = player->GetClient();
		client->QueuePacket(ClassicProtocol::MakeDisconnectPlayerPacket(Utils::MCString("Server shut down")));
		client->ProcessPacketsInQueue();
	}

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

	LOG(LOGLEVEL_INFO, "Client '%s' authorized with key %s", name.c_str(), packet.key.ToString().c_str());
	BroadcastMessage("&e" + name + " connected");

	Player::Ptr player = std::make_shared<Player>(client);

	m_linker.Add<Player>(player);
	client->SetId(player->GetId());
	player->SetName(name);

	client->QueuePacket(ClassicProtocol::MakeServerIdentificationPacket(ClassicProtocol::kVersion, m_serverName, m_serverMOTD, 0));
	client->SetAuthorized(true);

	if (packet.UNK0 == 0x42) {
		LOG(LOGLEVEL_DEBUG, "Client supports CPE, sending info.");
		player->SetCPEEnabled(true);
		client->QueuePacket(ExtendedProtocol::MakeExtInfoPacket(m_serverName, GetExtensionCount()));
		for (auto& entry : m_cpeEntries) {
			client->QueuePacket(ExtendedProtocol::MakeExtEntryPacket(Utils::MCString(entry.second.name), entry.second.version));
		}
	}

	m_worlds["default"]->AddPlayer(player);
}

void Server::OnSetBlockPacket(std::shared_ptr<Client> client, const ClassicProtocol::SetBlockPacket& packet)
{
	Player::Ptr player = m_linker.Get<Player>(client->GetId());
	player->GetWorld()->OnSetBlockPacket(player, packet);
}

void Server::OnPositionOrientationPacket(std::shared_ptr<Client> client, const ClassicProtocol::PositionOrientationPacket& packet)
{
	Player::Ptr player = m_linker.Get<Player>(client->GetId());
	player->GetWorld()->OnPositionOrientationPacket(player, packet);
}

void Server::OnMessagePacket(std::shared_ptr<Client> client, const ClassicProtocol::MessagePacket& packet)
{
	Player::Ptr player = m_linker.Get<Player>(client->GetId());
	std::string playerName = player->GetName();
	std::string message = playerName + ": " + packet.message.ToString();
	LOG(LOGLEVEL_NORMAL, message.c_str());
	BroadcastMessage(message);
}
