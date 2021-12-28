#include "../../include/Net/Client.hpp"

#include "../../include/Net/Packet.hpp"

using namespace Net;

int8_t Client::sid = 0;

class PartialPacket final : public Net::Packet
{
public:
	std::unique_ptr<Utils::BufferStream> bufferStream = nullptr;

	virtual void Deserialize(Utils::BufferStream& reader) override
	{
	}

	virtual std::unique_ptr<Utils::BufferStream> Serialize() override
	{
		return std::move(bufferStream);
	}
};

void Client::QueuePacket(std::shared_ptr<Net::Packet> packet)
{
	assert(packet != nullptr);
	m_packetQueue.push_back(packet);
}

void Client::ProcessPacketsInQueue()
{
	auto iter = m_packetQueue.begin();
	while (iter != m_packetQueue.end()) {
		auto bufferStreamPtr = (*iter)->Serialize();

		assert(bufferStreamPtr != nullptr);

		int result = m_socket->Send(*bufferStreamPtr);

		if (result < 0) {
			std::cerr << "Failed to send packet, result=" << result << std::endl;
			Kill();
			break;
		}

		// Partial packet, requeue remaining
		if (result < static_cast<int>((*iter)->GetSize())) {
			std::shared_ptr<PartialPacket> packet = std::make_shared<PartialPacket>();
			packet->bufferStream->Write(bufferStreamPtr->GetBufferPtr() + result, bufferStreamPtr->GetBufferSize() - result);
			*iter = packet;
			break;
		}

		iter = m_packetQueue.erase(iter);
	}
}
