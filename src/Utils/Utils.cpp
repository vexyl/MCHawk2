#include "../../include/Utils/Utils.hpp"

#include "../include/Server.hpp"

#include <cstring>
#include <cassert>
#include <zlib.h>

#ifdef _WIN32
#pragma comment(lib, "zlib.lib")
#endif

void Utils::CompressBuffer(const uint8_t* buffer, size_t bufferSize, uint8_t** outCompBuffer, size_t* outCompSize)
{
	assert(*outCompBuffer == nullptr && buffer != nullptr);

	*outCompBuffer = new uint8_t[bufferSize];
	if (*outCompBuffer == nullptr) {
		LOG(LOGLEVEL_ERROR, "Couldn't allocate memory for map buffer");
		std::exit(1);
	}

	std::memset(*outCompBuffer, 0, bufferSize);

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = (uLong)bufferSize;
	strm.next_in = (Bytef*)buffer;
	strm.avail_out = 0;
	strm.next_out = Z_NULL;

	int ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, (MAX_WBITS + 16), 8, Z_DEFAULT_STRATEGY);
	if (ret != Z_OK) {
		LOG(LOGLEVEL_ERROR, "Zlib error: deflateInit2()");
		std::exit(1);
	}

	strm.avail_out = (uLong)bufferSize;
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

Utils::Vector Utils::ConvertVectorToBlock(Utils::Vector& v)
{
	return Utils::Vector(v.x / 32, v.y / 32, v.z / 32);
}

Utils::Vector Utils::ConvertBlockToVector(Utils::Vector& pos)
{
	return Utils::Vector(pos.x * 32, pos.y * 32, pos.z * 32);
}

Utils::Vector Utils::ConvertBlockToPlayer(Utils::Vector& pos)
{
	return Utils::Vector(pos.x * 32, pos.y * 32 + 51, pos.z * 32);
}

Utils::Vector Utils::ConvertPlayerToBlock(Utils::Vector& pos)
{
	return Utils::Vector(pos.x / 32, (pos.y - 51) / 32, pos.z / 32);
}