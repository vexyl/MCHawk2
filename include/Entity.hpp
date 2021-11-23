#ifndef ENTITY_H_
#define ENTITY_H_

#include "Utils/Vector.hpp"

#include <stdint.h>

class Entity {
public:
	Entity() : m_pitch(0), m_yaw(0) {}

	Utils::Vector GetPosition() const { return m_position; }
	uint8_t GetPitch() const { return m_pitch; }
	uint8_t GetYaw() const { return m_yaw; }

	void SetPosition(const Utils::Vector& position) { m_position = position; }
	void SetVelocity(const Utils::Vector& velocity) { m_velocity = velocity; }
	void SetOrientation(uint8_t pitch, uint8_t yaw) { m_pitch = pitch;  m_yaw = yaw; }
	void SetPitch(uint8_t pitch) { m_pitch = pitch; }
	void SetYaw(uint8_t yaw) { m_yaw = yaw; }

protected:
	Utils::Vector m_position, m_velocity;
	uint8_t m_pitch, m_yaw;
};

#endif // ENTITY_H_