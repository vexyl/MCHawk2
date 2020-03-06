#ifndef PLAYER_H_
#define PLAYER_H_

#include "Net/Client.hpp"
#include "World.hpp"
#include "Position.hpp"

#include <string>

class Player final {
public:
	Player(Net::Client* client) : m_client(client), m_pitch(0), m_yaw(0) {}

	~Player() { delete m_client; }

	Player(const Player&) = delete;
	Player& operator=(const Player&) = delete;

	void SetWorld(World* world) { m_world = world; }
	World* GetWorld() { return m_world; }

	Net::Client* GetClient() { return m_client; }
	std::string GetName() const { return m_name; }
	Position GetPosition() const { return m_position; }
	uint8_t GetPitch() const { return m_pitch; }
	uint8_t GetYaw() const { return m_yaw; }

	void SetName(std::string name) { m_name = name; }
	void SetPosition(Position position) { m_position = position; }
	void SetOrientation(uint8_t pitch, uint8_t yaw) { m_pitch = pitch;  m_yaw = yaw; }

private:
	Net::Client* m_client = nullptr;
	std::string m_name;
	World* m_world = nullptr;
	Position m_position;
	uint8_t m_pitch, m_yaw;
};

#endif // PLAYER_H_