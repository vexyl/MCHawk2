#ifndef WORLD_H_
#define WORLD_H_

#include "Net/Client.hpp"
#include "Net/ClassicProtocol.hpp"
#include "Map.hpp"
#include "MapGen.hpp"
#include "Position.hpp"

#include <vector>

class World final {
public:
	World() : m_spawnPosition(0, 0, 0) {}

	~World() { m_map->SaveToFile("test.map"); }

	void Init()
	{
		//m_map = MapGen::GenerateFlatMap(256, 64, 256);
		m_map = std::make_unique<Map>();
		m_map->LoadFromFile("test.map", 256, 64, 256);
		if (!m_map->LoadFromFile("test.map", 256, 64, 256))
			m_map = MapGen::GenerateFlatMap(256, 64, 256);

		m_spawnPosition = { 256 / 2, 64 / 2, 256 / 2};
	}

	Map* GetMap() { return m_map.get(); }
	Position GetSpawnPosition() const { return m_spawnPosition; }
	void AddPlayer(int8_t pid);
	void RemovePlayer(int8_t pid);

	void Update();

	void SendLevel(Net::Client* client);

	void OnSetBlockPacket(Net::Client* client, const Net::ClassicProtocol::SetBlockPacket& packet);
	void OnPositionOrientationPacket(Net::Client* client, const Net::ClassicProtocol::PositionOrientationPacket& packet);

private:
	std::unique_ptr<Map> m_map;
	Position m_spawnPosition;
	std::vector<int8_t> m_playerPids;
};

#endif // WORLD_H_
