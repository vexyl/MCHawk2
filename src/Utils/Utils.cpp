#include "../../include/Utils/Utils.hpp"

#include <zlib.h>

#include <cstring>
#include <cassert>
#include <cstdlib>

#ifdef _WIN32
#pragma comment(lib, "zlib.lib")
#endif

namespace Utils {
// FIXME: Rewrite as loop to not need the outCompBuffer/outCompSize and return status (success, error)
void CompressBuffer(const uint8_t* buffer, std::size_t bufferSize, uint8_t** outCompBuffer, std::size_t* outCompSize)
{
	assert(*outCompBuffer == nullptr && buffer != nullptr);

	*outCompBuffer = new uint8_t[bufferSize];

	assert(*outCompBuffer != nullptr);

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
	assert(ret == Z_OK);

	strm.avail_out = (uLong)bufferSize;
	strm.next_out = (Bytef*)(*outCompBuffer);

	ret = deflate(&strm, Z_FINISH);

	switch (ret) {
	case Z_NEED_DICT:
	case Z_DATA_ERROR:
	case Z_MEM_ERROR:
		assert(false && "CompressBuffer error=" + std::to_string(ret));
		break;
	default:
		break;
	}

	deflateEnd(&strm);

	*outCompSize = (std::size_t)strm.total_out;
}

Vector ConvertVectorToBlock(Vector& v)
{
	return Vector(v.x / 32, v.y / 32, v.z / 32);
}

Vector ConvertBlockToVector(Vector& pos)
{
	return Vector(pos.x * 32, pos.y * 32, pos.z * 32);
}

Vector ConvertBlockToPlayer(Vector& pos)
{
	return Vector(pos.x * 32, pos.y * 32 + 51, pos.z * 32);
}

Vector ConvertPlayerToBlock(Vector& pos)
{
	return Vector(pos.x / 32, (pos.y - 51) / 32, pos.z / 32);
}
} // namespace Utils
