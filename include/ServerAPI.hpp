#ifndef SERVERAPI_H_
#define SERVERAPI_H_

#include "Net/Client.hpp"
#include "Position.hpp"

#include <string>

class Map;

namespace ServerAPI {
	bool CheckPrivilege(Net::Client* client, std::string priv);
	bool BroadcastMessage(Net::Client* srcClient, Net::Client* fromClient, std::string message);
	bool SendClientMessage(Net::Client* srcClient, Net::Client* dstClient, std::string message);
	bool MapSetBlock(Net::Client* srcClient, Map* map, Position pos, uint8_t type);
	bool SetUserType(Net::Client* client, Net::Client* dstClient, uint8_t type);
} // namespace ServerAPI

#endif // SERVERAPI_H_