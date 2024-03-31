#include "../../include/Utils/Utils.hpp"

namespace Utils {
Vector ConvertVectorToBlock(Vector& v)
{
	return Vector(v.x / 32, v.y / 32, v.z / 32);
}

Vector ConvertBlockToVector(Vector& pos)
{
	return Vector(pos.x * 32, pos.y * 32, pos.z * 32);
}

Vector ConvertBlockToPlayer(Vector& pos)
{
	return Vector(pos.x * 32, pos.y * 32 + 51, pos.z * 32);
}

Vector ConvertPlayerToBlock(Vector& pos)
{
	return Vector(pos.x / 32, (pos.y - 51) / 32, pos.z / 32);
}
} // namespace Utils
