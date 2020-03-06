#ifndef SERVERAPI_H_
#define SERVERAPI_H_

#include "Net/Client.hpp"
#include "Position.hpp"

#include <string>

class Map;

namespace ServerAPI {
	void BroadcastMessage(Net::Client* client, std::string message);
	void SendClientMessage(Net::Client* srcClient, Net::Client* dstClient, std::string message);
	void MapSetBlock(Net::Client* srcClient, Map* map, Position pos, uint8_t type);
} // namespace ServerAPI

#endif // SERVERAPI_H_