#include "../../include/Utils/MCString.hpp"

#include <cstring>

using namespace Utils;

MCString::MCString() : m_stringLength(0)
{
	InitializeFromString("");
}

MCString::MCString(const std::string& str)
{
	InitializeFromString(str);
}

// Copy constructor
MCString::MCString(const MCString& str)
{
	*this = str;
}

// Copy assignment
MCString& MCString::operator=(const MCString& str)
{
	std::memcpy(m_data, str.m_data, sizeof(m_data));
	Sanitize();
	return *this;
}

// Copy assignment
MCString& MCString::operator=(const std::string& str)
{
	InitializeFromString(str);
	return *this;
}

std::string MCString::ToString() const
{
	return std::string(reinterpret_cast<const char*>(m_data), m_stringLength);
}

void MCString::InitializeFromString(const std::string& str)
{
	size_t dataLength = sizeof(m_data);
	m_stringLength = str.length();

	if (m_stringLength < dataLength) {
		std::memset(&m_data[m_stringLength], 0x20, dataLength - m_stringLength);
		std::memcpy(m_data, str.c_str(), m_stringLength);
	} else {
		std::memcpy(m_data, str.c_str(), dataLength);
		m_stringLength = dataLength;
	}
}

void MCString::InitializeFromRawBuffer(const std::uint8_t(&buffer)[64])
{
	std::memcpy(m_data, buffer, 64);
	Sanitize();
}

void MCString::Sanitize()
{
	// Default string length; used if loop below doesn't find padding
	m_stringLength = 64;
	int paddingIndex = -1;

	// Minecraft classic string padding
	for (int i = 0; i < sizeof(m_data); ++i) {
		if (m_data[i] == 0x20 || m_data[i] == 0x00) {
			if (paddingIndex == -1)
				paddingIndex = i;
		} else {
			paddingIndex = -1;
		}
	}

	if (paddingIndex >= 0)
		m_stringLength = static_cast<size_t>(paddingIndex);
}
