#ifndef SERVER_H_
#define SERVER_H_

#include "Net/Packet.hpp"
#include "Utils/MCString.hpp"
#include "Net/ProtocolHandler.hpp"
#include "Net/ClassicProtocol.hpp"
#include "Net/ExtendedProtocol.hpp"
#include "Utils/Logger.hpp"
#include "Net/TCPSocket.hpp"
#include "Net/Client.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "CPEEntry.hpp"

#include <vector>
#include <map>

#pragma region LoggerMacros
#define LOG(...) Server::GetInstance()->GetLogger().Log(__VA_ARGS__);
#define LOGLEVEL_NORMAL Utils::Logger::LogLevel::kNormal
#define LOGLEVEL_DEBUG Utils::Logger::LogLevel::kDebug
#define LOGLEVEL_INFO Utils::Logger::LogLevel::kInfo
#define LOGLEVEL_WARNING Utils::Logger::LogLevel::kWarning
#define LOGLEVEL_ERROR Utils::Logger::LogLevel::kError
#pragma endregion

class Server final {
public:
	Server() : m_socket(true), m_logger("log.txt"), m_blockDefaultEventHandler(false) {};

	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	~Server();

	static Server* GetInstance();

	Player::PlayerPtr GetPlayer(uint8_t pid);

	Utils::Logger& GetLogger() { return m_logger; }
	Net::ProtocolHandler& GetProtocolHandler() { return m_protocolHandler; }

	std::shared_ptr<World> GetWorld(std::string name)
	{
		std::shared_ptr<World> world;
		auto iter = m_worlds.find(name);
		if (iter != m_worlds.end())
			world = iter->second;
		return world;
	}

	std::map<std::string, std::shared_ptr<World>> GetWorlds()
	{
		return m_worlds;
	}

	uint8_t GetCPEEntryVersion(std::string name) const
	{
		uint8_t version = 0;
		auto search = m_cpeEntries.find(name);
		if (search != m_cpeEntries.end()) {
			return search->second.version;
		}

		return version;
	}

	uint16_t GetExtensionCount() const { return static_cast<uint16_t>(m_cpeEntries.size()); }

	void Init();

	static void SendClientMessage(Net::Client* client, std::string message, int messageType = 0);
	static void SendWrappedMessage(Net::Client* client, std::string message, int messageType = 0);
	void BroadcastMessage(std::string message, int messageType = 0);

	bool Update();

	void Shutdown();

private:
	static Server* thisPtr; // Singleton

	Utils::Logger m_logger;
	Net::ProtocolHandler m_protocolHandler;
	Net::TCPSocket m_socket;

	bool m_running = true;

	std::vector<Net::Client*> m_unauthorizedClients;
	std::map<int8_t, std::shared_ptr<Player>> m_players;
	std::map<std::string, std::shared_ptr<World>> m_worlds;
	std::map<std::string, CPEEntry> m_cpeEntries;

	std::string m_serverName, m_serverMOTD;

	bool m_blockDefaultEventHandler;

	void ProcessUnauthorizedClients();
	void UpdatePlayers();
	void CheckForConnections();

	void AddCPEEntry(std::string name, uint8_t version);

	void OnAuthenticationPacket(Net::Client* client, const Net::ClassicProtocol::AuthenticationPacket& packet);
	void OnSetBlockPacket(Net::Client* client, const Net::ClassicProtocol::SetBlockPacket& packet);
	void OnPositionOrientationPacket(Net::Client* client, const Net::ClassicProtocol::PositionOrientationPacket& packet);
	void OnMessagePacket(Net::Client* client, const Net::ClassicProtocol::MessagePacket& packet);
};

std::shared_ptr<World> MakeDefaultWorld();

#endif // SERVER_H_
