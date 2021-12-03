#include "../../include/Utils/Utils.hpp"

Utils::Vector Utils::ConvertVectorToBlock(Utils::Vector& v)
{
	return Utils::Vector(v.x / 32, v.y / 32, v.z / 32);
}

Utils::Vector Utils::ConvertBlockToVector(Utils::Vector& pos)
{
	return Utils::Vector(pos.x * 32, pos.y * 32, pos.z * 32);
}

Utils::Vector Utils::ConvertBlockToPlayer(Utils::Vector& pos)
{
	return Utils::Vector(pos.x * 32, pos.y * 32 + 51, pos.z * 32);
}

Utils::Vector Utils::ConvertPlayerToBlock(Utils::Vector& pos)
{
	return Utils::Vector(pos.x / 32, (pos.y - 51) / 32, pos.z / 32);
}