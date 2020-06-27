#ifndef PRIVILEGES_H_
#define PRIVILEGES_H_

#include <map>
#include <vector>
#include <string>

// TODO: Have groups of privs and cache them too

struct priv_result {
		std::string message;
		int error;
};

class PrivilegeHandler final {
public:
	void GivePrivilege(std::string name, std::string priv);
	// RemovePrivilege()

	priv_result HasPrivilege(std::string name, std::string priv);
	// FlushPrivilegeCache

private:
	std::map<std::string, std::vector<std::string>> m_privs;
};

#endif // PRIVILEGES_H_