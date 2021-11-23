#include "../../include/Net/Client.hpp"

#include "../../include/Net/Packet.hpp"

using namespace Net;

int8_t Client::sid = 0;

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

		if (result < static_cast<int>((*iter)->GetSize())) {
			std::cerr << "Sent partial message" << std::endl;
			std::exit(1);
		}

		iter = m_packetQueue.erase(iter);
	}
}
