#include "../include/Player.hpp"
#include "../include/ServerAPI.hpp"
#include "../include/Net/ClassicProtocol.hpp"
#include "../include/Net/ExtendedProtocol.hpp"

#undef SendMessage // bug with winsock

void Player::SetHotbarSlot(uint8_t index, uint8_t blockType)
{
	// Not checking if block is already in the slot because there's
	// no way to know which slot the player put a block in
	if (index < kMaxHotbarSlots) {
		m_hotbar[index] = blockType;
		if (HasCPEEntry("HeldBlock", 1))
			m_client->QueuePacket(Net::ExtendedProtocol::MakeSetHotbarPacket(blockType, index));
	}
}

void Player::SetInventoryOrder(uint8_t order, uint8_t blockType)
{
	if (HasCPEEntry("InventoryOrder", 1))
		m_client->QueuePacket(Net::ExtendedProtocol::MakeSetInventoryOrderPacket(order, blockType));
}

bool Player::HasCPEEntry(std::string name, int version) const
{
	auto search = m_cpeEntries.find(name);
	if (search != m_cpeEntries.end() && search->second.version == version)
		return true;
	return false;
}

void Player::AddCPEEntry(std::string name, uint8_t version)
{
	if (HasCPEEntry(name, version)) {
		std::cout << "failed to add CPEExt " << name << " version " << version << ", already added" << std::endl;
		return;
	}

	m_cpeEntries[name] = { name, version };
}

void Player::SendMessage(std::string message)
{
	ServerAPI::SendClientMessage(nullptr, GetClient(), message);
}