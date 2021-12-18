#ifndef WORLD_H_
#define WORLD_H_

#include "Net/Client.hpp"
#include "Net/ClassicProtocol.hpp"
#include "Map.hpp"
#include "MapGen.hpp"
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

	World() : m_spawnPosition(0, 0, 0), m_weatherType(WeatherType::kSunny) {}

	World(std::string name) : World()
	{
		m_name = name;
	}

	Map* GetMap() { return m_map.get(); }
	Utils::Vector GetSpawnPosition() const { return m_spawnPosition; }
	std::string GetName() const { return m_name; }
	const std::vector<Player::PlayerPtr>& GetPlayers() const { return m_players; }
	WeatherType GetWeatherType() const { return m_weatherType; }

	void SetMap(std::unique_ptr<Map> map) { m_map = std::move(map); }
	void SetSpawnPosition(const Utils::Vector& position) { m_spawnPosition = position; }
	void SetWeatherType(WeatherType type);

	void AddPlayer(Player::PlayerPtr player);
	void RemovePlayer(int8_t pid);

	void Update();

	void SendLevel(Net::Client* client);
	void SendWeatherType(Player::PlayerPtr player);
	void SendBlockDefinitions(Player::PlayerPtr player);

	void OnSetBlockPacket(Player::PlayerPtr player, const Net::ClassicProtocol::SetBlockPacket& packet);
	void OnPositionOrientationPacket(Player::PlayerPtr player, const Net::ClassicProtocol::PositionOrientationPacket& packet);

private:
	static int8_t pid;

	std::unique_ptr<Map> m_map;
	std::string m_name;
	Utils::Vector m_spawnPosition;
	std::vector<Player::PlayerPtr> m_players;
	WeatherType m_weatherType;
	std::vector<BlockDef> m_blockDefinitions;
};

#endif // WORLD_H_
