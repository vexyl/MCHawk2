#include "SendLevelTask.h"

int MapDeflateContext::Initialize(const uint8_t* buffer_in, std::size_t buffer_in_size)
{
	if (buffer_in == nullptr || buffer_in_size == 0)
		return 0;
	assert(buffer_in != nullptr && buffer_in_size != 0);

	bufferIn = buffer_in;
	bufferInSize = buffer_in_size;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.total_in = 0;
	strm.avail_out = 0;
	strm.next_out = Z_NULL;

	lastOut = 0;
	flush = Z_NO_FLUSH;

	bufferOut = new uint8_t[bufferInSize];
	std::memset(bufferOut, 0, bufferInSize);

	int ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, (MAX_WBITS + 16), 8, Z_DEFAULT_STRATEGY);
	return ret == Z_OK ? 1 : 0;
}

int MapDeflateContext::CompressNextChunk()
{
	assert(bufferIn != nullptr && bufferInSize != 0);

	int lastTotalIn = 0;

	strm.avail_in = std::min(kChunkSize, (unsigned int)(bufferInSize - strm.total_in));
	strm.next_in = (Bytef*)bufferIn + strm.total_in;

	if (strm.total_in + strm.avail_in == bufferInSize)
		flush = Z_FINISH;

	do {
		strm.avail_out = kChunkSize;
		strm.next_out = (Bytef*)(bufferOut + strm.total_out);

		int flushStream = flush;
		if (flushStream != Z_FINISH && (strm.total_out - lastOut) >= kChunkSize) {
			flushStream = Z_FULL_FLUSH;
			lastOut = strm.total_out;
		}

		int ret = deflate(&strm, flush != Z_FINISH ? Z_FULL_FLUSH : flush);

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

void SendLevelTask::OnInit()
{
	Task::OnInit();
	
	m_client->SetTemporaryPacketQueue(true);
	
	auto levelInitializePacket = Net::ClassicProtocol::MakeLevelInitializePacket();

	std::shared_ptr<Map> map = m_world->GetMap();
	int ret = m_mapDeflateContext.Initialize(map->GetReadOnlyBufferPtr(), map->GetBufferSize());

	assert(ret != 0);

	m_compBuffer = m_mapDeflateContext.bufferOut;
	m_client->QueuePacket(levelInitializePacket, true);
}

void SendLevelTask::OnFinished()
{
	//LOG(LOGLEVEL_DEBUG, "Compressed map size: %d bytes", mapDeflateContext.bufferOutSize);

	delete m_compBuffer;

	std::shared_ptr<Map> map = m_world->GetMap();
	auto levelFinalizePacket = Net::ClassicProtocol::MakeLevelFinalizePacket(map->GetXSize(), map->GetYSize(), map->GetZSize());

	m_client->QueuePacket(levelFinalizePacket, true);
	m_client->SetTemporaryPacketQueue(false);
}

void SendLevelTask::OnFailed()
{
	delete m_compBuffer;
}

void SendLevelTask::OnUpdate()
{
	if (!m_client->KeepAlive()) {
		Fail();
		return;
	}

	assert(m_compBuffer != nullptr);

	int ret = m_mapDeflateContext.CompressNextChunk();

	assert(ret != 0 && "Failed to compress map");

	size_t compSize = m_mapDeflateContext.strm.total_out;
	while (m_bytes < compSize) {
		size_t remaining_bytes = compSize - m_bytes;
		size_t count = 1024;
		if (remaining_bytes < 1024 && ret < 0)
			break;

		auto chunkPacket = std::make_shared<Net::ClassicProtocol::LevelDataChunkPacket>();

		chunkPacket->chunkLength = static_cast<uint16_t>(count);

		std::memcpy(chunkPacket->chunkData, &m_compBuffer[m_bytes], count);

		// Padding; must send exactly 1024 m_bytes per chunk
		if (count < 1024) {
			size_t paddingSize = 1024 - count;
			std::memset(&chunkPacket->chunkData[count], 0x00, paddingSize);
		}

		m_bytes += count;
		chunkPacket->percent = static_cast<uint8_t>((((float)m_mapDeflateContext.strm.total_in / (float)m_mapDeflateContext.bufferInSize) * 100.0f));

		m_client->QueuePacket(chunkPacket, true);
		m_client->ProcessPacketsInQueue();
	}

	if (ret > 0)
		Finish();
}
