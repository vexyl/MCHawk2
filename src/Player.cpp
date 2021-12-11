#include "../include/Player.hpp"
#include "../include/ServerAPI.hpp"
#include "../include/Net/ClassicProtocol.hpp"

#undef SendMessage

void Player::SendMessage(std::string message)
{
	ServerAPI::SendClientMessage(nullptr, GetClient(), message);
}


void Player::AddCPEEntry(std::string name, uint8_t version)
{
	if (HasCPEEntry(name, version)) {
		std::cout << "failed to add CPEExt " << name << " version " << version << ", already added" << std::endl;
		return;
	}

	m_cpeEntries[name] = { name, version };
}

bool Player::HasCPEEntry(std::string name, int version) const
{
	auto search = m_cpeEntries.find(name);
	if (search != m_cpeEntries.end() && search->second.version == version)
		return true;
	return false;
}
