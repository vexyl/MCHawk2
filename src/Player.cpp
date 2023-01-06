#include "../include/Player.hpp"
#include "../include/Net/ClassicProtocol.hpp"
#include "../include/Net/ExtendedProtocol.hpp"

#include <algorithm>

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
	auto result = std::find_if(m_cpeEntries.begin(), m_cpeEntries.end(), [version](const auto& val) { return val.second.version == version; });
	return result != m_cpeEntries.end();
}

void Player::AddCPEEntry(std::string name, uint8_t version)
{
	if (!HasCPEEntry(name, version))
		m_cpeEntries[name] = { name, version };
}
