#ifndef POSITION_H_
#define POSITION_H_

#include "../Utils/Vector.hpp"

#include <cstdint>

namespace Block {
struct Position final {
	static Position ConvertVectorToBlock(Utils::Vector& v)
	{
		return Position(static_cast<int16_t>(v.x) / 32, static_cast<int16_t>(v.y) / 32, static_cast<int16_t>(v.z) / 32);
	}

	static Utils::Vector ConvertBlockToVector(Position& pos)
	{
		return Utils::Vector(static_cast<float>(pos.x) * 32, static_cast<float>(pos.y) * 32, static_cast<float>(pos.z) * 32);
	}

	static Position ConvertBlockToPlayer(Position& pos)
	{
		return Position(pos.x * 32, pos.y * 32 + 51, pos.z * 32);
	}

	static Position ConvertPlayerToBlock(Position& pos)
	{
		return Position(pos.x / 32, (pos.y - 51) / 32, pos.z / 32);
	}

	Position(int16_t x, int16_t y, int16_t z);
	Position();

	// Copy constructor
	Position(const Position& pos);

	// Copy assignment
	Position& operator=(const Position& pos);

	Utils::Vector ToVector()
	{
		return Utils::Vector(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	}

	int16_t x, y, z;
};
} // namespace Block

#endif // POSITION_H_
