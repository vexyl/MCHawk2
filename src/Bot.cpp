#include "../../include/Bot.hpp"
#include "../../include/Net/ClassicProtocol.hpp"

#include <math.h>

#define PI 3.14159265 // FIXME: Put in a Utils math namespace

Bot::Bot(std::string name) : Entity(), m_name(name), m_lastYaw(0), m_lastPitch(0)
{

}

bool Bot::ChangedPosition() const
{
	return (m_lastPosition.x != m_position.x || m_lastPosition.y != m_position.y || m_lastPosition.z != m_position.z);
}

bool Bot::ChangedOrientation() const
{
	return (m_lastYaw != m_yaw || m_lastPitch != m_pitch);
}

void Bot::FacePosition(Utils::Vector targetPosition)
{
	Utils::Vector facingVec = targetPosition - m_position;
	float angle = static_cast<float>(atan2(facingVec.z, facingVec.x) * 180.0 / PI);
	angle = 255 - (192 - (angle / 360) * 255);
	SetYaw(static_cast<uint8_t>(angle));
}

void Bot::FaceEntity(const Entity& entity)
{
	FacePosition(entity.GetPosition());
}

void Bot::Update()
{
	m_lastYaw = m_yaw;
	m_lastPitch = m_pitch;
	m_lastPosition = m_position;
}
