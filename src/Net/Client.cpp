#include "Client.hpp"

#include "Packet.hpp"

#include <iostream>

using namespace Net;

int8_t Client::sid = 0;

bool Client::TrySocketReceive(size_t packetSize, Utils::BufferStream& reader) const
{
	return m_socket->Receive(reader);
}

void Client::QueuePacket(std::shared_ptr<Net::Packet> packet, bool useTemporaryQueue)
{
	assert(packet != nullptr);

	if (useTemporaryQueue)
		m_temporaryPacketQueue.push_back(packet);
	else
		m_packetQueue.push_back(packet);
}

void Client::SetTemporaryPacketQueue(bool useTemporaryQueue)
{
	if (m_useTemporaryQueue && !useTemporaryQueue)
		FlushTemporaryPacketQueue();
	else if (!m_useTemporaryQueue && useTemporaryQueue)
		ProcessPacketsInQueue();
	m_useTemporaryQueue = useTemporaryQueue;
}

void Client::FlushTemporaryPacketQueue()
{
	for (auto& packet : m_temporaryPacketQueue) {
		m_packetQueue.push_front(packet);
	}
	m_temporaryPacketQueue.clear();
}

void Client::ProcessPacketsInQueue()
{
	std::list<std::shared_ptr<Net::Packet>>* packetQueue = &m_packetQueue;
	if (m_useTemporaryQueue)
		packetQueue = &m_temporaryPacketQueue;

	auto iter = packetQueue->begin();
	while (iter != packetQueue->end()) {
		if (!m_keepAlive) {
			packetQueue->clear();
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
			std::shared_ptr<PartialPacket> packet = std::make_shared<PartialPacket>(bufferStreamPtr->GetBufferPtr() + result, packetSize);
			*iter = packet;
			break;
		}

		iter = packetQueue->erase(iter);
	}
}
