#ifndef WORLD_H_
#define WORLD_H_

#include "IServer.hpp"
#include "Net/Client.hpp"
#include "Net/ClassicProtocol.hpp"
#include "Map.hpp"
#include "MapGen.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Vector.hpp"
#include "Player.hpp"
#include "BlockDef.hpp"
#include "Utils/Logger.hpp"

#include <vector>
#include <memory>

class World final {
public:
	enum class WeatherType {
		kSunny = 0,
		kRaining,
		kSnowing
	};

	enum class EnvironmentType {
		kSkyColor = 0,
		kCloudColor,
		kFogColor,
		kAmbientLight,
		kDiffuseLight,
		kSkyboxColor
	};

	struct environment_setting {
		Utils::Color color;
		bool modified = false;
	};

	struct environment {
		environment_setting skyColor;
		environment_setting cloudColor;
		environment_setting fogColor;
		environment_setting ambientLight;
		environment_setting diffuseLight;
		environment_setting skyboxColor;
	};

	World() = delete;
	World(IServer& server, const Utils::Logger::Ptr& logger) : m_server(server), m_logger(logger), m_spawnPosition(0, 0, 0), m_weatherType(WeatherType::kSunny) {}
	World(IServer& server, const Utils::Logger::Ptr& logger, std::string name) : m_server(server), m_logger(logger), m_name(name), m_weatherType(WeatherType::kSunny) {}

	std::shared_ptr<Map> GetMap() { return m_map; }
	Utils::Vector GetSpawnPosition() const { return m_spawnPosition; }
	std::string GetName() const { return m_name; }
	const std::vector<Player::PlayerPtr>& GetPlayers() const { return m_players; }
	WeatherType GetWeatherType() const { return m_weatherType; }

	void SetMap(std::shared_ptr<Map> map) { m_map = std::move(map); }
	void SetSpawnPosition(const Utils::Vector& position) { m_spawnPosition = position; }
	void SetEnvironmentSetting(EnvironmentType type, Utils::Color color);
	void SetWeatherType(WeatherType type);

	void AddPlayer(Player::PlayerPtr player);
	void RemovePlayer(int8_t pid);

	void AddBlockDef(BlockDef def);

	void Update();

	void SendLevel(std::shared_ptr<Net::Client> client);
	void SendWeatherType(Player::PlayerPtr player);
	void SendBlockDefinitions(Player::PlayerPtr player);
	void SendBlockPermissions(Player::PlayerPtr player);

	void OnSetBlockPacket(Player::PlayerPtr player, const Net::ClassicProtocol::SetBlockPacket& packet);
	void OnPositionOrientationPacket(Player::PlayerPtr player, const Net::ClassicProtocol::PositionOrientationPacket& packet);

private:
	static int8_t pid;

	IServer& m_server;
	Utils::Logger::Ptr m_logger;
	std::shared_ptr<Map> m_map;
	std::string m_name;
	Utils::Vector m_spawnPosition;
	std::vector<Player::PlayerPtr> m_players;
	WeatherType m_weatherType;
	std::vector<BlockDef> m_blockDefinitions;
	environment m_env;
};

#endif // WORLD_H_
