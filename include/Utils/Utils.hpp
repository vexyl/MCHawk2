#ifndef UTILS_H_
#define UTILS_H_

#include "Vector.hpp"

namespace Utils {
Utils::Vector ConvertVectorToBlock(Utils::Vector& v);
Utils::Vector ConvertBlockToVector(Utils::Vector& pos);
Utils::Vector ConvertBlockToPlayer(Utils::Vector& pos);
Utils::Vector ConvertPlayerToBlock(Utils::Vector& pos);
} // namespace Utils

#endif // UTILS_H_