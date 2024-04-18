#ifndef SENDLEVEL_TASK_H_
#define SENDLEVEL_TASK_H_

#include "../Task.h"
#include "../World.h"

struct MapDeflateContext {
	const unsigned int kChunkSize = 1024 * 1024;

	z_stream strm;
	int flush;

	const uint8_t* bufferIn;
	std::size_t bufferInSize;
	uint8_t* bufferOut;
	std::size_t bufferOutSize;
	std::size_t lastOut;

	int Initialize(const uint8_t* buffer_in, std::size_t buffer_in_size);
	int CompressNextChunk();
};

class SendLevelTask final : public Task {
public:
	explicit SendLevelTask(World* world, std::shared_ptr<Net::Client> client) : m_world(world), m_client(client)
	{
		assert(m_world != nullptr && m_client != nullptr);
	}

private:
	World* m_world;
	std::shared_ptr<Net::Client> m_client;
	MapDeflateContext m_mapDeflateContext;
	uint8_t* m_compBuffer = nullptr;
	size_t m_bytes = 0;

protected:
	virtual void OnInit() override;
	virtual void OnFinished() override;
	virtual void OnFailed() override;
	virtual void OnUpdate() override;
};

#endif // SENDLEVEL_TASK_H_