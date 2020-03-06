#ifndef MAPGEN_H_
#define MAPGEN_H_

#include <memory>

class Map;

struct MapGen final {
	static std::unique_ptr<Map> GenerateFlatMap(uint16_t x, uint16_t y, uint16_t z);
};

#endif // MAPGEN_H_