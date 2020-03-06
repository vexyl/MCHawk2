void SendMap(Net::Socket& socket)
{
	ClassicProtocol::LevelInitializePacket levelInitializePacket;
	socket.Send(*levelInitializePacket.Serialize());

	uint8_t* compBuffer = nullptr;
	size_t compSize;

	CompressBuffer(&compBuffer, &compSize);

	//LOG(DEBUG, "Compressed map size: %d bytes", compSize);

	size_t bytes = 0;
	while (bytes < compSize) {
		size_t remainingBytes = compSize - bytes;
		size_t count = (remainingBytes >= 1024) ? 1024 : (remainingBytes);

		ClassicProtocol::LevelDataChunkPacket chunkPacket;

		chunkPacket.chunkLength = static_cast<uint16_t>(count);

		memcpy(chunkPacket.chunkData, &compBuffer[bytes], count);

		// Padding; must send exactly 1024 bytes per chunk
		if (count < 1024) {
			size_t paddingSize = 1024 - count;
			std::memset(&chunkPacket.chunkData[count], 0x00, paddingSize);
		}

		bytes += count;

		std::cout << bytes << "/" << compSize << " bytes" << std::endl;

		chunkPacket.percent = static_cast<uint8_t>((((float)bytes / (float)compSize) * 100.0f));

		std::cout << (int)chunkPacket.percent << "%" << std::endl;
		auto result = socket.Send(*chunkPacket.Serialize());
		if (result != 1028)
			std::cout << "Failed to send whole message; sent " << result << " bytes" << std::endl;
	}

	free(compBuffer);

	ClassicProtocol::LevelFinalizePacket levelFinalizePacket;

	levelFinalizePacket.x = 256;
	levelFinalizePacket.y = 64;
	levelFinalizePacket.z = 256;

	socket.Send(*levelFinalizePacket.Serialize());
}

#define calcMapOffset(x, y, z, xSize, zSize) ((y * zSize + z) * xSize + x) + 4

void CompressBuffer(uint8_t** outCompBuffer, size_t* outCompSize)
{
	assert(*outCompBuffer == nullptr && m_buffer != nullptr);

	*outCompBuffer = (uint8_t*)std::malloc(sizeof(uint8_t) * m_bufferSize);
	if (*outCompBuffer == nullptr) {
		LOG(DEBUG, "Couldn't allocate memory for map buffer");
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
		LOG(DEBUG, "Zlib error: deflateInit2()");
		std::exit(1);
	}

	strm.avail_out = (uLong)m_bufferSize;
	strm.next_out = (Bytef*)(*outCompBuffer);

	ret = deflate(&strm, Z_FINISH);

	switch (ret) {
	case Z_NEED_DICT:
	case Z_DATA_ERROR:
	case Z_MEM_ERROR:
		LOG(DEBUG, "Zlib error: inflate()");
		std::exit(1);
	}

	deflateEnd(&strm);

	*outCompSize = (size_t)strm.total_out;
}


void GenerateFlatMap(short x, short y, short z)
{
	m_bufferSize = x * y * z + 4;
	m_buffer = (uint8_t*)std::malloc(sizeof(uint8_t) * m_bufferSize);

	std::memset(m_buffer, 0, m_bufferSize);
	 
	int sz = htonl(m_bufferSize - 4);
	std::memcpy(m_buffer, &sz, sizeof(sz));

	for (short gen_y = 0; gen_y < y / 2; gen_y++) {
		for (short gen_x = 0; gen_x < x; gen_x++) {
			for (short gen_z = 0; gen_z < z; gen_z++) {
				int index = calcMapOffset(gen_x, gen_y, gen_z, x, z);
				if (gen_y < (y / 2 - 1))
					m_buffer[index] = 0x03;
				else
					m_buffer[index] = 0x02;
			}
		}
	}

	LOG(DEBUG, "Generated flat map");
	std::cout << "bufferSize=" << m_bufferSize << std::endl;
}