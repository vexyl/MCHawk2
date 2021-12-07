#include "../include/Player.hpp"
#include "../include/ServerAPI.hpp"

void Player::SendMessage(std::string message)
{
	ServerAPI::SendClientMessage(nullptr, GetClient(), message);
}

void Player::AddCPEExtension(std::string name, int version)
{
	if (HasCPEExtension(name, version)) {
		std::cout << "failed to add CPEExt " << name << " version " << version << ", already added" << std::endl;
		return;
	}

	m_cpeExtensions[name] = { name, version };
}

bool Player::HasCPEExtension(std::string name, int version) const
{
	auto search = m_cpeExtensions.find(name);
	if (search != m_cpeExtensions.end() && search->second.version == version)
		return true;
	return false;
}
