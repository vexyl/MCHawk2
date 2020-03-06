#ifndef MCSTRING_H_
#define MCSTRING_H_

#include <string>

namespace Utils {
class MCString final {
public:
	friend class BufferStream;

	MCString();
	MCString(const std::string& str);

	~MCString() {}

	MCString(const MCString&);
	MCString& operator=(const MCString&);
	MCString& operator=(const std::string&);

	size_t GetLength() const { return m_stringLength; }
	uint8_t* GetBufferPtr() { return m_data; }

	std::string ToString() const;

private:
	uint8_t m_data[64];
	size_t m_stringLength = 0;

	void InitializeFromRawBuffer(const std::uint8_t(&buffer)[64]);
	void InitializeFromString(const std::string& str);
	void Sanitize();
};
} // namespace Utils

#endif // MCSTRING_H_