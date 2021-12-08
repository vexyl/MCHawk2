#include "../include/Map.hpp"

#include <iostream>
#include <fstream>

uint8_t Map::PeekBlock(Utils::Vector& pos)
{
	int offset = calcMapOffset(static_cast<int>(pos.x), static_cast<int>(pos.y), static_cast<int>(pos.z), m_xSize, m_zSize);

	if (offset < 0 || offset >= (int)m_bufferSize) {
		//std::cerr << "Buffer overflow in Map::PeekBlock()" << std::endl;
		//std::exit(1);
		return 0; // FIXME - should this exit or throw error?
	}

	return m_buffer[offset];
}

void Map::SetBlock(Utils::Vector& pos, uint8_t type)
{
	int offset = calcMapOffset(static_cast<int>(pos.x), static_cast<int>(pos.y), static_cast<int>(pos.z), m_xSize, m_zSize);

	if (offset < 0 || offset >= (int)m_bufferSize) {
		std::cerr << "Buffer overflow in Map::SetBlock()" << std::endl;
		std::exit(1);
	}

	m_buffer[offset] = type;
}

void Map::SaveToFile(std::string filename)
{
	std::ofstream file(filename, std::ios::binary);
	if (!file.write(reinterpret_cast<const char*>(m_buffer), static_cast<std::streamsize>(m_bufferSize))) {
		std::cerr << "Failed to save map to file" << std::endl;
		std::exit(1);
	}
}

// FIXME: Map dimensions should not be set here
bool Map::LoadFromFile(std::string filename, uint16_t x, uint16_t y, uint16_t z)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file) return false;

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	m_bufferSize = static_cast<size_t>(size);
	m_buffer = new uint8_t[m_bufferSize];
	if (m_buffer == nullptr) {
		std::cerr << "Failed to allocate memory for map buffer" << std::endl;
		std::exit(1);
	}

	if (!file.read(reinterpret_cast<char*>(m_buffer), size)) {
		std::cerr << "Failed to load map from file" << std::endl;
		std::exit(1);
	}

	m_xSize = x;
	m_ySize = y;
	m_zSize = z;

	return true;
}
