#include "../include/ServerAPI.hpp"

#include "../include/Server.hpp"
//#include "../include/Map.hpp"

using namespace Net;

void ServerAPI::PrefixMessage(Net::Client* client, std::string& message, bool hideConsolePrefix)
{
		if (client != nullptr) {
				Player::PlayerPtr player = Server::GetInstance()->GetPlayer(client->GetID());
				message = "[" + player->GetName() + "] " + message;
		}
		else {
				if (hideConsolePrefix == false)
					message = "[CONSOLE] " + message;
		}
}

// TODO: return priv_result so plugins can send message instead
bool ServerAPI::CheckPrivilege(Net::Client* client, std::string priv)
{
	if (client == nullptr)
		return true;

	Server* server = Server::GetInstance();
	Player::PlayerPtr player = server->GetPlayer(client->GetID());

	assert(player != nullptr);
	
	auto privResult = server->GetPrivilegeHandler().HasPrivilege(player->GetName(), priv);
	if (privResult.error) {
		SendClientMessage(nullptr, client, privResult.message);
		std::string message = "Client " + std::to_string(client->GetID()) + " (" + client->GetIPAddress() + ") " + privResult.message;
		LOG(LOGLEVEL_DEBUG, message.c_str());
		return false;
	}

	return true;
}

bool ServerAPI::BroadcastMessage(Client* srcClient, Client* fromClient, std::string message, bool hideConsolePrefix)
{
	if (!CheckPrivilege(srcClient, "BroadcastMessage"))
		return false;

	if (srcClient == nullptr && !CheckPrivilege(fromClient, "chat"))
		return false;

	if (srcClient == nullptr && fromClient != nullptr) {
		PrefixMessage(fromClient, message);
	} else {
		if (!hideConsolePrefix)
			message = "&e" + message;
		PrefixMessage(nullptr, message, hideConsolePrefix);
	}

	// ClassicalSharp color codes
	// and this could be made more efficient by keeping track of pos of last color
	size_t colorPos = 0;
	while ((colorPos = message.find('%', colorPos)) != std::string::npos) {
		if (colorPos == message.size() - 1)
			break;

		char c = message.at(colorPos + 1);
		if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
			message[colorPos] = '&';

		colorPos++;
	}

    LOG(LOGLEVEL_NORMAL, message.c_str());

	Server::GetInstance()->BroadcastMessage(message);
	return true;
}

bool ServerAPI::SendClientMessage(Client* srcClient, Client* dstClient, std::string message, bool hideConsolePrefix)
{
	assert(dstClient != nullptr);

	if (!CheckPrivilege(srcClient, "SendClientMessage"))
		return false;

	if (!hideConsolePrefix)
		message = "&e" + message;
	PrefixMessage(srcClient, message, hideConsolePrefix);

	Server::SendWrappedMessage(dstClient, message);
	return true;
}

bool ServerAPI::MapSetBlock(Net::Client* client, Map* map, Position pos, uint8_t type)
{
	//std::string message = "placed block " + std::to_string(type) + " (" + ClassicProtocol::GetBlockNameByType(type) + ") @ (" + std::to_string(static_cast<short>(pos.x)) + ", " + std::to_string(static_cast<short>(pos.y)) + ", " + std::to_string(static_cast<short>(pos.z)) + ")";

	//PrefixMessage(client, message);
	//LOG(LOGLEVEL_DEBUG, message.c_str());

	if (!CheckPrivilege(client, "MapSetBlock")) {
		uint8_t actualType = map->PeekBlock(pos);
		client->QueuePacket(ClassicProtocol::MakeSetBlock2Packet(pos.x, pos.y, pos.z, actualType));
		return false;
	}

	map->SetBlock(pos, type);
	return true;
}

bool ServerAPI::SetUserType(Net::Client* srcClient, Net::Client* dstClient, uint8_t type)
{
	if (!CheckPrivilege(srcClient, "SetUserType"))
		return false;

	std::string message = "set user type of sid " + std::to_string(dstClient->GetID()) + " to " + std::to_string(type);
	PrefixMessage(srcClient, message, false);
	LOG(LOGLEVEL_DEBUG, message.c_str());
	dstClient->QueuePacket(ClassicProtocol::MakeUserTypePacket(type));

	return true;
}
