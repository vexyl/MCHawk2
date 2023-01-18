#ifndef MAPGEN_H_
#define MAPGEN_H_

#include <memory>

class Map;

struct MapGen final {
	static std::shared_ptr<Map> GenerateFlatMap(uint16_t xSize, uint16_t ySize, uint16_t zSize);
};

#endif // MAPGEN_H_