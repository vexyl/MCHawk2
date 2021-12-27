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
} // namespace Utils

#endif // UTILS_H_
