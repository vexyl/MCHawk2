#include "../../include/Net/Client.hpp"

#include "../../include/Net/Packet.hpp"

using namespace Net;

int8_t Client::sid = 0;

void Client::UseTemporaryPacketQueue(bool use)
{
	if (!use)
		FlushTemporaryPacketQueue();
	m_useTemporaryPacketQueue = use;
}

void Client::QueuePacket(std::shared_ptr<Net::Packet> packet)
{
	assert(packet != nullptr);
	if (m_useTemporaryPacketQueue)
		m_temporaryPacketQueue.push_back(packet);
	else
		m_packetQueue.push_back(packet);
}

void Client::QueuePacketForcePrimary(std::shared_ptr<Net::Packet> packet)
{
	assert(packet != nullptr);
	m_packetQueue.push_back(packet);
}

void Client::FlushTemporaryPacketQueue()
{
	for (auto& packet : m_temporaryPacketQueue)
		m_packetQueue.push_back(packet);
	m_temporaryPacketQueue.clear();
}

void Client::ProcessPacketsInQueue(bool forcePrimaryQueue)
{
	if (m_useTemporaryPacketQueue && !forcePrimaryQueue)
		return;

	auto iter = m_packetQueue.begin();
	while (iter != m_packetQueue.end()) {
		if (!m_keepAlive) {
			m_packetQueue.clear();
			return;
		}

		auto bufferStreamPtr = (*iter)->Serialize();

		assert(bufferStreamPtr != nullptr);

		int result = m_socket->Send(*bufferStreamPtr);
		if (result < 0) {
			if (errno != EWOULDBLOCK) {
				std::cerr << "Failed to send packet with " << bufferStreamPtr->GetBufferSize() << " bytes, result=" << result << ", errno=" << errno << std::endl;
				Kill();
			}

			break;
		}

		// Partial packet, requeue remaining
		if (result < static_cast<int>((*iter)->GetSize())) {
			size_t packetSize = bufferStreamPtr->GetBufferSize() - result;
			//std::cerr << "partial packet, queueing remaining " << packetSize << " bytes" << std::endl;
			std::shared_ptr<PartialPacket> packet = std::make_shared<PartialPacket>(bufferStreamPtr->GetBufferPtr() + result, packetSize);
			*iter = packet;
			break;
		}

		iter = m_packetQueue.erase(iter);
	}
}
