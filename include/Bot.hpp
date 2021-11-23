#ifndef BOT_H_
#define BOT_H_

#include "Entity.hpp"

#include <string>

class Bot : public Entity {
public:
	Bot(std::string name);

	std::string GetName() const { return m_name; }

	bool ChangedPosition() const;
	bool ChangedOrientation() const;

	void FacePosition(Utils::Vector targetPosition);
	void FaceEntity(const Entity& entity);

	void Update();

private:
	std::string m_name;
	Utils::Vector m_lastPosition;
	uint8_t m_lastYaw, m_lastPitch;
};

#endif // BOT_H_