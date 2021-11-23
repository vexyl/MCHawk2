#ifndef PLAYER_H_
#define PLAYER_H_

#include "Entity.hpp"
#include "Net/Client.hpp"
#include "Utils/Vector.hpp"

#include <string>

class World;

class Player final : public Entity {
public:
	typedef std::shared_ptr<Player> PlayerPtr;

	Player(Net::Client* client) : Entity(), m_client(client) { }

	~Player() { delete m_client; } // FIXME

	Player(const Player::PlayerPtr) = delete;
	Player::PlayerPtr operator=(const Player::PlayerPtr) = delete;

	void SetWorld(World* world) { m_world = world; }
	World* GetWorld() { return m_world; }

	Net::Client* GetClient() { return m_client; }
	std::string GetName() const { return m_name; }
	int8_t GetID() const { return m_pid; }

	void SetName(std::string name) { m_name = name; }
	void SetID(int8_t pid) { m_pid = pid; }
	void SetPosition(const Utils::Vector& position) { m_position = position; }

	void SendMessage(std::string message);

private:
	int8_t m_pid = -1;
	Net::Client* m_client = nullptr;
	std::string m_name;
	World* m_world = nullptr;
};

#endif // PLAYER_H_
