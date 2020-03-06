#include "../include/ServerAPI.hpp"

#include "../include/Server.hpp"
//#include "../include/Map.hpp"

using namespace Net;

#pragma region HelperMacros
#define SERVERAPI_PREFIX_MESSAGE(client, message) \
	Server* serverAPI_prefix_server = Server::GetInstance(); \
	if (client != nullptr) { \
		Player& player = serverAPI_prefix_server->GetPlayer(client->GetPID()); \
		message = "[" + player.GetName() + "] " + message; \
	} else { \
		message = "&e[CONSOLE] " + message; \
	}
#pragma endregion

void ServerAPI::BroadcastMessage(Client* client, std::string message)
{
	SERVERAPI_PREFIX_MESSAGE(client, message);

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

	if (client != nullptr)
		LOG(LOGLEVEL_NORMAL, message.c_str());

	serverAPI_prefix_server->BroadcastMessage(message);
}

void ServerAPI::SendClientMessage(Client* srcClient, Client* dstClient, std::string message)
{
	assert(dstClient != nullptr);
	SERVERAPI_PREFIX_MESSAGE(srcClient, message);
	Server::SendClientMessage(dstClient, message);
}

void ServerAPI::MapSetBlock(Net::Client* client, Map* map, Position pos, uint8_t type)
{
	// std::string message = "placed block " + std::to_string(type) + " @ (" + std::to_string(static_cast<short>(pos.x)) + ", " + std::to_string(static_cast<short>(pos.y)) + ", " + std::to_string(static_cast<short>(pos.z)) + ")";

	//SERVERAPI_PREFIX_MESSAGE(client, message);
	//LOG(LOGLEVEL_DEBUG, message.c_str());

	map->SetBlock(pos, type);
}