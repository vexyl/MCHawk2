#ifndef ENTITY_H_
#define ENTITY_H_

#include "Utils/Vector.hpp"

#include <string>
#include <stdint.h>

class Entity {
public:
	Entity() : m_pitch(0), m_yaw(0), m_lastPitch(0), m_lastYaw(0) {}

	std::string GetName() const { return m_name; }
	Utils::Vector GetPosition() const { return m_position; }
	uint8_t GetPitch() const { return m_pitch; }
	uint8_t GetYaw() const { return m_yaw; }

	void SetName(std::string name) { m_name = name; }
	void SetPosition(const Utils::Vector& position) { m_lastPosition = position; m_position = position; }
	void SetVelocity(const Utils::Vector& velocity) { m_velocity = velocity; }
	void SetOrientation(uint8_t pitch, uint8_t yaw) { m_pitch = pitch;  m_yaw = yaw; }
	void SetPitch(uint8_t pitch) { m_pitch = pitch; }
	void SetYaw(uint8_t yaw) { m_yaw = yaw; }

protected:
	std::string m_name;
	std::string skinName;
	Utils::Vector m_position, m_lastPosition, m_velocity;
	uint8_t m_pitch, m_yaw, m_lastPitch, m_lastYaw;
};

#endif // ENTITY_H_