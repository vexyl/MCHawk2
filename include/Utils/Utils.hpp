#ifndef UTILS_H_
#define UTILS_H_

#include "Vector.hpp"

#include <cstddef>
#include <stdint.h>
#include <algorithm>

#include <cstring>
#include <cassert>

#include <zlib.h>

namespace Utils {
struct MapDeflateContext {
	const unsigned int kChunkSize = 1024;

	z_stream strm;
	int flush;

	const uint8_t* bufferIn;
	std::size_t bufferInSize;
	uint8_t* bufferOut;
	std::size_t bufferOutSize;

	int Initialize(const uint8_t* buffer_in, std::size_t buffer_in_size)
	{
		if (buffer_in == nullptr || buffer_in_size == 0)
			return 0;

		bufferIn = buffer_in;
		bufferInSize = buffer_in_size;

		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_out = 0;
		strm.next_out = Z_NULL;

		flush = Z_NO_FLUSH;

		bufferOut = new uint8_t[bufferInSize];
		std::memset(bufferOut, 0, bufferInSize);

		int ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, (MAX_WBITS + 16), 8, Z_DEFAULT_STRATEGY);
		return ret == Z_OK ? 1 : 0;
	}

	int CompressNextChunk()
	{
		assert(bufferIn != nullptr && bufferInSize != 0);

		strm.avail_in = std::min(kChunkSize, (unsigned int)(bufferInSize - strm.total_in));
		strm.next_in = (Bytef*)bufferIn + strm.total_in;

		if (strm.total_in + strm.avail_in == bufferInSize)
			flush = Z_FINISH;

		do {
			strm.avail_out = kChunkSize;
			strm.next_out = (Bytef*)(bufferOut + strm.total_out);

			int ret = deflate(&strm, flush);

			switch (ret) {
			case Z_NEED_DICT:
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
			case Z_STREAM_ERROR:
				deflateEnd(&strm);
				return 0;
				break;
			default:
				break;
			}
		} while (strm.avail_out == 0);

		if (flush == Z_FINISH) {
			bufferOutSize = strm.total_out;
			deflateEnd(&strm);
			if (bufferOut == nullptr || bufferOutSize == 0)
				return 0;
			return 1;
		}
		return -1;
	}
};

Vector ConvertVectorToBlock(Vector& v);
Vector ConvertBlockToVector(Vector& pos);
Vector ConvertBlockToPlayer(Vector& pos);
Vector ConvertPlayerToBlock(Vector& pos);

struct Color {
	uint8_t red, green, blue;

	Color(uint8_t r, uint8_t g, uint8_t b)
	{
		this->red = r;
		this->green = g;
		this->blue = b;
	}

	Color() : Color(255, 255, 255) {}
	
	Color(const Color& color)
	{
		this->red = color.red;
		this->green = color.green;
		this->blue = color.blue;
	}

	Color operator=(const Color& color)
	{
		this->red = color.red;
		this->green = color.green;
		this->blue = color.blue;

		return *this;
	}
};
} // namespace Utils

#endif // UTILS_H_
