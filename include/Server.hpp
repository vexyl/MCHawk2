#ifndef SERVER_H_
#define SERVER_H_

#include "IServer.hpp"
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
#include <memory>

class Server final : public IServer {
public:
	Server();
	
	~Server();

	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	Player::PlayerPtr GetPlayer(uint8_t pid);

	virtual Net::ProtocolHandler& GetProtocolHandler() override { return m_protocolHandler; }

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

	virtual uint8_t GetCPEEntryVersion(std::string name) const override
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

	static void SendClientMessage(std::shared_ptr<Net::Client> client, std::string message, int messageType = 0);
	static void SendWrappedMessage(std::shared_ptr<Net::Client> client, std::string message, int messageType = 0);
	void BroadcastMessage(std::string message, int messageType = 0);

	bool Update();

	void Shutdown();

private:
	Utils::Logger::Ptr m_logger;
	Net::ProtocolHandler m_protocolHandler;
	Net::TCPSocket m_socket;

	bool m_running = true;

	std::vector<std::shared_ptr<Net::Client>> m_unauthorizedClients;
	std::map<int8_t, std::shared_ptr<Player>> m_players;
	std::map<std::string, std::shared_ptr<World>> m_worlds;
	std::map<std::string, CPEEntry> m_cpeEntries;

	std::string m_serverName, m_serverMOTD;

	bool m_blockDefaultEventHandler;

	void ProcessUnauthorizedClients();
	void UpdatePlayers();
	void CheckForConnections();

	void AddCPEEntry(std::string name, uint8_t version);

	void OnAuthenticationPacket(std::shared_ptr<Net::Client> client, const Net::ClassicProtocol::AuthenticationPacket& packet);
	void OnSetBlockPacket(std::shared_ptr<Net::Client> client, const Net::ClassicProtocol::SetBlockPacket& packet);
	void OnPositionOrientationPacket(std::shared_ptr<Net::Client> client, const Net::ClassicProtocol::PositionOrientationPacket& packet);
	void OnMessagePacket(std::shared_ptr<Net::Client> client, const Net::ClassicProtocol::MessagePacket& packet);
};

std::shared_ptr<World> MakeDefaultWorld(IServer& server, const Utils::Logger::Ptr& logger);

#endif // SERVER_H_
