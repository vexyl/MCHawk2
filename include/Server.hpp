#ifndef SERVER_H_
#define SERVER_H_

#include "Net/Packet.hpp"
#include "Utils/MCString.hpp"
#include "Net/ProtocolHandler.hpp"
#include "Net/ClassicProtocol.hpp"
#include "Utils/Logger.hpp"
#include "Net/TCPSocket.hpp"
#include "Net/Client.hpp"
#include "World.hpp"
#include "Player.hpp"

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
	Server() : m_socket(true), m_logger("log.txt") {};

	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	~Server();

	static Server* GetInstance();

	Player& GetPlayer(uint8_t pid);
	Utils::Logger& GetLogger() { return m_logger; }

	void Init();

	static void SendClientMessage(Net::Client* client, std::string message);
	static void SendWrappedMessage(Net::Client* client, std::string message);
	void BroadcastMessage(std::string message);

	bool Update();
	
	void Shutdown();

private:
	static Server* thisPtr; // Singleton

	Utils::Logger m_logger;
	Net::ProtocolHandler m_protocolHandler;
	Net::TCPSocket m_socket;
	bool m_running = true;

	std::vector<Net::Client*> m_unauthorizedClients;
	std::map<int8_t, Player> m_players;

	World m_world;

	void OnAuthenticationPacket(Net::Client* client, const Net::ClassicProtocol::AuthenticationPacket& packet);
	void OnSetBlockPacket(Net::Client* client, const Net::ClassicProtocol::SetBlockPacket& packet);
	void OnPositionOrientationPacket(Net::Client* client, const Net::ClassicProtocol::PositionOrientationPacket& packet);
	void OnMessagePacket(Net::Client* client, const Net::ClassicProtocol::MessagePacket& packet);

	void ProcessUnauthorizedClients();
	void UpdatePlayers();
	void CheckForConnections();
};

#endif // SERVER_H_
