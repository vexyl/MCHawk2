#ifdef DEBUG
#ifndef FILESOCKET_H_
#define FILESOCKET_H_

// For testing purposes

#include "Socket.hpp"

namespace MC {
namespace Net {
class FileSocket final : public Socket {
public:
	FileSocket(std::string filename);

	virtual ~FileSocket() override;

	virtual void Bind(std::string ipAddress, short port) override;
	virtual Socket* Accept() const override;
	virtual size_t Poll() override;
	virtual int Receive(uint8_t *outData, size_t outDataSize) override;
	virtual int Send(const BufferWriter& writer) override;

private:
	uint8_t* m_buffer = nullptr;
	size_t m_bufferSize = 0;
	size_t m_bytesReadFromFile = 0;
};
} // namespace MC::Net
} // namespace MC

#endif // FILESOCKET_H_
#endif // DEBUG