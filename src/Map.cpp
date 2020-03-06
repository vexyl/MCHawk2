#include "../include/Map.hpp"

#include "../include/Server.hpp"

#include <cstring>
#include <zlib.h>

void Map::CompressBuffer(uint8_t** outCompBuffer, size_t* outCompSize) const
{
	assert(*outCompBuffer == nullptr && m_buffer != nullptr);

	*outCompBuffer = new uint8_t[m_bufferSize];
	if (*outCompBuffer == nullptr) {
		LOG(LOGLEVEL_ERROR, "Couldn't allocate memory for map buffer");
		std::exit(1);
	}

	std::memset(*outCompBuffer, 0, m_bufferSize);

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = (uLong)m_bufferSize;
	strm.next_in = (Bytef*)m_buffer;
	strm.avail_out = 0;
	strm.next_out = Z_NULL;

	int ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, (MAX_WBITS + 16), 8, Z_DEFAULT_STRATEGY);
	if (ret != Z_OK) {
		LOG(LOGLEVEL_ERROR, "Zlib error: deflateInit2()");
		std::exit(1);
	}

	strm.avail_out = (uLong)m_bufferSize;
	strm.next_out = (Bytef*)(*outCompBuffer);

	ret = deflate(&strm, Z_FINISH);

	switch (ret) {
	case Z_NEED_DICT:
	case Z_DATA_ERROR:
	case Z_MEM_ERROR:
		LOG(LOGLEVEL_ERROR, "Zlib error: inflate()");
		std::exit(1);
	}

	deflateEnd(&strm);

	*outCompSize = (size_t)strm.total_out;
}

void Map::SetBlock(Position& pos, uint8_t type)
{
	int offset = calcMapOffset(pos.x, pos.y, pos.z, m_xSize, m_zSize);

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
void Map::LoadFromFile(std::string filename, uint16_t x, uint16_t y, uint16_t z)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
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
}
