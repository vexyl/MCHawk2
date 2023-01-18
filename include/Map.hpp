#ifndef MAP_H_
#define MAP_H_

#include "Utils/Vector.hpp"

#include <string>
#include <cstdint>

int PositionToIndex(int x, int y, int z, int xSize, int zSize);

class Map final {
public:
	Map() {}

	~Map() { delete m_buffer; }

	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;

	const uint8_t* GetReadOnlyBufferPtr() const { return m_buffer; }
	size_t GetBufferSize() const { return m_bufferSize; }
	uint8_t PeekBlock(Utils::Vector& pos);
	uint16_t GetXSize() const { return m_xSize; }
	uint16_t GetYSize() const { return m_ySize; }
	uint16_t GetZSize() const { return m_zSize; }

	void SetBlock(Utils::Vector& pos, uint8_t type);

	void SaveToFile(std::string filename);
	bool LoadFromFile(std::string filename, uint16_t x, uint16_t y, uint16_t z);

private:
	friend struct MapGen;

	uint8_t* m_buffer = nullptr;
	size_t m_bufferSize = 0;
	uint16_t m_xSize = 0, m_ySize = 0, m_zSize = 0;

	int PositionToIndex(int x, int y, int z);
};

#endif // MAP_H_
