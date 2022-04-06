#ifndef WORLD_H_
#define WORLD_H_

#include "Net/Client.hpp"
#include "Net/ClassicProtocol.hpp"
#include "Map.hpp"
#include "MapGen.hpp"
#include "Utils/Utils.hpp"
#include "Utils/Vector.hpp"
#include "Player.hpp"
#include "BlockDef.hpp"

#include <vector>

class World final {
public:
	enum WeatherType {
		kSunny = 0,
		kRaining,
		kSnowing
	};

	enum EnvironmentType {
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

	void World::SetEnvironmentSetting(EnvironmentType type, Utils::Color color)
	{
		switch (type) {
		case EnvironmentType::kSkyColor:
			m_env.skyColor.color = color;
			break;
		case EnvironmentType::kCloudColor:
			m_env.cloudColor.color = color;
			break;
		case EnvironmentType::kFogColor:
			m_env.fogColor.color = color;
			break;
		case EnvironmentType::kAmbientLight:
			m_env.ambientLight.color = color;
			break;
		case EnvironmentType::kDiffuseLight:
			m_env.diffuseLight.color = color;
			break;
		case EnvironmentType::kSkyboxColor:
			m_env.skyboxColor.color = color;
			break;
		}
	}

	World() : m_spawnPosition(0, 0, 0), m_weatherType(WeatherType::kSunny) {}

	World(std::string name) : World()
	{
		m_name = name;
	}

	std::shared_ptr<Map> GetMap() { return m_map; }
	Utils::Vector GetSpawnPosition() const { return m_spawnPosition; }
	std::string GetName() const { return m_name; }
	const std::vector<Player::PlayerPtr>& GetPlayers() const { return m_players; }
	WeatherType GetWeatherType() const { return m_weatherType; }

	void SetMap(std::shared_ptr<Map> map) { m_map = std::move(map); }
	void SetSpawnPosition(const Utils::Vector& position) { m_spawnPosition = position; }
	void SetWeatherType(WeatherType type);

	void AddPlayer(Player::PlayerPtr player);
	void RemovePlayer(int8_t pid);

	void AddBlockDef(BlockDef def);

	void Update();

	void SendLevel(Net::Client* client);
	void SendWeatherType(Player::PlayerPtr player);
	void SendBlockDefinitions(Player::PlayerPtr player);
	void SendBlockPermissions(Player::PlayerPtr player);

	void OnSetBlockPacket(Player::PlayerPtr player, const Net::ClassicProtocol::SetBlockPacket& packet);
	void OnPositionOrientationPacket(Player::PlayerPtr player, const Net::ClassicProtocol::PositionOrientationPacket& packet);

	void NewBlockDef(
		uint8_t blockID,
		std::string name,
		uint8_t solidity,
		uint8_t movementSpeed,
		uint8_t topTextureID, uint8_t sideTextureID, uint8_t bottomTextureID,
		uint8_t transmitLight,
		uint8_t walkSound,
		uint8_t fullBright,
		uint8_t shape,
		uint8_t blockDraw,
		uint8_t fogDensity,
		uint8_t fogR, uint8_t fogG, uint8_t fogB
	);

private:
	static int8_t pid;

	std::shared_ptr<Map> m_map;
	std::string m_name;
	Utils::Vector m_spawnPosition;
	std::vector<Player::PlayerPtr> m_players;
	WeatherType m_weatherType;
	std::vector<BlockDef> m_blockDefinitions;
	environment m_env;
};

#endif // WORLD_H_
