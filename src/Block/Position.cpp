#include "../../include/BLock/Position.hpp"

using namespace Block;

Position::Position(int16_t x, int16_t y, int16_t z) :
	x(x), y(y), z(z)
{}

Position::Position() : Position(0, 0, 0) {}

// Copy constructor
Position::Position(const Position& pos)
{
	*this = pos;
}

// Copy assignment
Position& Position::operator=(const Position& pos)
{
	x = pos.x;
	y = pos.y;
	z = pos.z;

	return *this;
}