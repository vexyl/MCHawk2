#include "SendLevelTask.h"

void SendLevelTask::OnInit()
{
	Task::OnInit();

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