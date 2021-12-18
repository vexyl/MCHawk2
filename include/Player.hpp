#ifndef PLAYER_H_
#define PLAYER_H_

#include "Entity.hpp"
#include "Net/Client.hpp"
#include "CPEEntry.hpp"
#include "Utils/Vector.hpp"

#include <string>
#include <map>

class World;

class Player final : public Entity {
public:
	typedef std::shared_ptr<Player> PlayerPtr;
	static const int kMaxHotbarSlots = 9;

	int heldBlock = 0;

	Player(Net::Client* client) : Entity(), m_client(client)
	{
		for (int i = 0; i < kMaxHotbarSlots; ++i)
			m_hotbar[i] = -1;
	}

	~Player() { delete m_client; } // FIXME

	Player(const Player::PlayerPtr) = delete;
	Player::PlayerPtr operator=(const Player::PlayerPtr) = delete;

	void SetWorld(World* world) { m_world = world; }
	World* GetWorld() { return m_world; }

	Net::Client* GetClient() { return m_client; }
	std::string GetName() const { return m_name; }
	int8_t GetID() const { return m_pid; }
	uint8_t GetHeldBlock() const { return heldBlock; }

	void SetName(std::string name) { m_name = name; }
	void SetID(int8_t pid) { m_pid = pid; }
	void SetHotbarSlot(uint8_t index, uint8_t blockType);

	bool HasCPEEntry(std::string name, int version) const;
	void AddCPEEntry(std::string name, uint8_t version);

	void SendMessage(std::string message);

	std::map<std::string, CPEEntry> GetCPEEntries() const { return m_cpeEntries; }

private:
	int8_t m_pid = -1;
	Net::Client* m_client = nullptr;
	std::string m_name;
	std::map<std::string, CPEEntry> m_cpeEntries;
	World* m_world = nullptr;
	int m_hotbar[9];
};

#endif // PLAYER_H_
