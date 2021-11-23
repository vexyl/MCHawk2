#include "../include/Player.hpp"
#include "../include/ServerAPI.hpp"

void Player::SendMessage(std::string message)
{
	ServerAPI::SendClientMessage(nullptr, GetClient(), message);
}