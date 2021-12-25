#include "../include/Privileges.hpp"

void PrivilegeHandler::GivePrivilege(std::string name, std::string priv)
{
	auto iter = m_privs.find(name);
	if (iter == m_privs.end()) {
		// TODO: error check
		m_privs.insert(std::make_pair(name, std::vector { priv }));
	}
	else {
		// TODO: Check if already has priv
		iter->second.push_back(priv);
	}
}

void PrivilegeHandler::TakePrivilege(std::string name, std::string priv)
{
	auto iter = m_privs.find(name);
	if (iter != m_privs.end()) {
		std::remove_if(iter->second.begin(), iter->second.end(), [&](std::string checkPriv) { return priv == checkPriv; });
	}
}

priv_result PrivilegeHandler::HasPrivilege(std::string name, std::string priv) const
{
	priv_result result{ "Missing priv: " + priv, -1 };

	auto iter = m_privs.find(name);
	if (iter != m_privs.end()) {
		for (auto& obj : iter->second) {
			if (obj == priv) {
				result.error = 0;
				break;
			}
		}
	}

	return result;
}