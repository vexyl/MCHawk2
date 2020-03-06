#ifdef DEBUG
#include "FileSocket.hpp"

// For testing purposes

using namespace MC::Net;

FileSocket::FileSocket(std::string filename)
{
	std::ifstream file(filename.c_str(), std::ios_base::binary | std::ios_base::ate);
	if (!file) {
		std::cout << "Failed to open file: " << filename << std::endl;
		std::exit(1);
	}

	auto fileSize = file.tellg();
	file.seekg(std::ios::beg);

	m_bufferSize = static_cast<size_t>(fileSize);
	m_buffer = new uint8_t[m_bufferSize];
	if (m_buffer == nullptr) {
		std::cerr << "Failed to poll socket: not enough memory" << std::endl;
		std::exit(1);
	}

	file.read(reinterpret_cast<char*>(m_buffer), fileSize);
	file.close();
}

FileSocket::~FileSocket()
{
	delete m_buffer;
}

void FileSocket::Bind(std::string ipAddress, short port)
{

}

Socket* FileSocket::Accept() const
{
	return nullptr;
}

size_t FileSocket::Poll()
{
	size_t numBytesRemaining = m_bufferSize - m_bytesReadFromFile;

	if (numBytesRemaining > 0) {
		// Confusing name...bytesRead for a Write() command!
		auto bytesRead = m_socketBuffer.Write(&m_buffer[m_bytesReadFromFile], numBytesRemaining);

		m_bytesReadFromFile += bytesRead;

		//std::cout << "Poll() - " << bytesRead << " received | total read=" << m_bytesReadFromFile << std::endl;
	}

	return m_socketBuffer.GetAvailableDataCount(); // Bytes available to read in buffer
}

int FileSocket::Receive(uint8_t *outData, size_t outDataSize)
{
	if (!m_socketBuffer.Read(outData, outDataSize))
		return -1;
	else
		return outDataSize;
}

int FileSocket::Send(const BufferWriter& writer)
{
	return -1;
}
#endif // DEBUG