#include "../../include/Utils/Utils.hpp"

#include <iostream>

#include <cassert>
#include <cstdlib>

namespace Utils {
// FIXME: Rewrite as loop to not need the outCompBuffer/outCompSize and return status (success, error)
void CompressBuffer(const uint8_t* buffer, std::size_t bufferSize, uint8_t** outCompBuffer, std::size_t* outCompSize)
{
	const unsigned int kChunkSize = 1024;

	assert(*outCompBuffer == nullptr && buffer != nullptr);

	*outCompBuffer = new uint8_t[bufferSize];

	assert(*outCompBuffer != nullptr);

	std::memset(*outCompBuffer, 0, bufferSize);

	int ret;
	int flush = Z_NO_FLUSH;

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_out = 0;
	strm.next_out = Z_NULL;

	ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, (MAX_WBITS + 16), 8, Z_DEFAULT_STRATEGY);

	assert(ret == Z_OK);

	do {
		strm.avail_in = std::min(kChunkSize, (unsigned int)(bufferSize - strm.total_in));
		strm.next_in = (Bytef*)buffer + strm.total_in;

		if (strm.total_in + strm.avail_in == bufferSize)
			flush = Z_FINISH;

		do {
			strm.avail_out = kChunkSize;
			strm.next_out = (Bytef*)(*outCompBuffer + strm.total_out);

			ret = deflate(&strm, flush);

			switch (ret) {
			case Z_NEED_DICT:
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
			case Z_STREAM_ERROR:
				std::cerr << "CompressBuffer error=" << ret << std::endl;
				assert(false);
				break;
			default:
				break;
			}
		} while (strm.avail_out == 0);
	} while (flush != Z_FINISH);

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
