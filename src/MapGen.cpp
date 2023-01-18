#include "../include/MapGen.hpp"

#include "../include/Map.hpp"

#include <cstring>

// FIXME: only used for htonl
#ifdef __linux__
#include <arpa/inet.h>
#elif _WIN32
#define NOMINMAX // bug with winsock redefining min
#include <winsock2.h>
#undef NOMINMAX
#endif

std::shared_ptr<Map> MapGen::GenerateFlatMap(uint16_t xSize, uint16_t ySize, uint16_t zSize)
{
	auto map = std::make_shared<Map>();

	map->m_bufferSize = static_cast<size_t>(xSize * ySize * zSize) + 4;
	map->m_buffer = new uint8_t[map->m_bufferSize];

	std::memset(map->m_buffer, 0, map->m_bufferSize);

	int32_t sz = htonl(static_cast<unsigned long>(map->m_bufferSize) - 4);
	std::memcpy(map->m_buffer, &sz, sizeof(sz));

	for (int16_t gen_y = 0; gen_y < ySize / 2; gen_y++) {
		for (int16_t gen_x = 0; gen_x < xSize; gen_x++) {
			for (int16_t gen_z = 0; gen_z < zSize; gen_z++) {
				int index = PositionToIndex(gen_x, gen_y, gen_z, xSize, zSize);
				if (gen_y < (ySize / 2 - 1))
					map->m_buffer[index] = 0x03;
				else
					map->m_buffer[index] = 0x02;
			}
		}
	}

	map->m_xSize = xSize;
	map->m_ySize = ySize;
	map->m_zSize = zSize;

	return std::move(map);
}
