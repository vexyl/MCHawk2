#ifndef MAP_H_
#define MAP_H_

#include "Position.hpp"

#include <string>

#include <cstdint>

#define calcMapOffset(x, y, z, xSize, zSize) ((y * zSize + z) * xSize + x) + 4

class Map final {
public:
	Map() {}

	~Map() { delete m_buffer; }

	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;

	uint16_t GetXSize() const { return m_xSize; }
	uint16_t GetYSize() const { return m_ySize; }
	uint16_t GetZSize() const { return m_zSize; }

	void SetBlock(Position& pos, uint8_t type);

	void CompressBuffer(uint8_t** outCompBuffer, size_t* outCompSize) const;
	void SaveToFile(std::string filename);
	void LoadFromFile(std::string filename, uint16_t x, uint16_t y, uint16_t z);

private:
	friend struct MapGen;

	uint8_t* m_buffer = nullptr;
	size_t m_bufferSize = 0;
	uint16_t m_xSize = 0, m_ySize = 0, m_zSize = 0;
};

#endif // MAP_H_