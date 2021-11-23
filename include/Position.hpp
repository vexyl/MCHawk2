#ifndef POSITION_H_
#define POSITION_H_

#include "Utils/Vector.hpp"

#include <cstdint>

struct Position final {
	Position(int16_t x, int16_t y, int16_t z) :
		x(x), y(y), z(z)
	{}

	Position() : Position(0, 0, 0) {}

	// Copy constructor
	Position(const Position& pos)
	{
		*this = pos;
	}

	// Copy constructor
	Position(const Utils::Vector& v)
	{
		*this = v;
	}

	// Copy assignment
	Position& operator=(const Position& pos)
	{
		x = pos.x;
		y = pos.y;
		z = pos.z;

		return *this;
	}

	// Copy assignment
	Position& operator=(const Utils::Vector& v)
	{
		x = static_cast<int16_t>(v.x * 32);
		y = static_cast<int16_t>(v.y * 32);
		z = static_cast<int16_t>(v.z * 32);

		return *this;
	}

	int16_t x, y, z;
};

#endif // POSITION_H_