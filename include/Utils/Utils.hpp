#ifndef UTILS_H_
#define UTILS_H_

#include "Vector.hpp"

#include <stdint.h>

namespace Utils {
void CompressBuffer(const uint8_t* buffer, size_t bufferSize, uint8_t** outCompBuffer, size_t* outCompSize);

Utils::Vector ConvertVectorToBlock(Utils::Vector& v);
Utils::Vector ConvertBlockToVector(Utils::Vector& pos);
Utils::Vector ConvertBlockToPlayer(Utils::Vector& pos);
Utils::Vector ConvertPlayerToBlock(Utils::Vector& pos);
} // namespace Utils

#endif // UTILS_H_