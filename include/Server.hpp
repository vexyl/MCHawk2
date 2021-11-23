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
#include "Privileges.hpp"

#include <vector>

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
	PrivilegeHandler& GetPrivilegeHandler() { return m_privHandler; }
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

	void BlockDefaultEventHandler(bool blocked = true) { m_blockDefaultEventHandler = blocked; }
	bool IsDefaultEventHandlerBlocked() const { return m_blockDefaultEventHandler; }

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
	PrivilegeHandler m_privHandler;

	bool m_running = true;

	std::vector<Net::Client*> m_unauthorizedClients;
	std::map<int8_t, std::shared_ptr<Player>> m_players; // FIXME: changed to shared_ptr

	std::map<std::string, std::shared_ptr<World>> m_worlds;

	bool m_blockDefaultEventHandler;

	void OnAuthenticationPacket(Net::Client* client, const Net::ClassicProtocol::AuthenticationPacket& packet);
	void OnSetBlockPacket(Net::Client* client, const Net::ClassicProtocol::SetBlockPacket& packet);
	void OnPositionOrientationPacket(Net::Client* client, const Net::ClassicProtocol::PositionOrientationPacket& packet);
	void OnMessagePacket(Net::Client* client, const Net::ClassicProtocol::MessagePacket& packet);

	void ProcessUnauthorizedClients();
	void UpdatePlayers();
	void CheckForConnections();
};

#endif // SERVER_H_
