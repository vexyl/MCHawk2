#ifndef UTILS_H_
#define UTILS_H_

#include "Vector.hpp"

#include <cstddef>
#include <stdint.h>

namespace Utils {
void CompressBuffer(const uint8_t* buffer, std::size_t bufferSize, uint8_t** outCompBuffer, std::size_t* outCompSize);

Vector ConvertVectorToBlock(Vector& v);
Vector ConvertBlockToVector(Vector& pos);
Vector ConvertBlockToPlayer(Vector& pos);
Vector ConvertPlayerToBlock(Vector& pos);

struct Color {
	uint8_t red, green, blue;

	Color(uint8_t r, uint8_t g, uint8_t b)
	{
		this->red = r;
		this->green = g;
		this->blue = b;
	}

	Color() : Color(255, 255, 255) {}
	
	Color(const Color& color)
	{
		this->red = color.red;
		this->green = color.green;
		this->blue = color.blue;
	}

	Color operator=(const Color& color)
	{
		this->red = color.red;
		this->green = color.green;
		this->blue = color.blue;
		return *this;
	}
};
} // namespace Utils

#endif // UTILS_H_
