#ifndef LUAPLUGINS_H_
#define LUAPLUGINS_H_

#include "ServerAPI.hpp"
#include "Net/Client.hpp"
#include "Player.hpp"
#include "Utils/Vector.hpp"

#include <sol/sol.hpp>
#include <memory>

#pragma comment(lib, "LuaLib.lib") 

class IPlugin {
public:
	virtual void Init() = 0;
	virtual void Tick() = 0;
	virtual std::string GetName() = 0;
};

class LuaPlugin final : public IPlugin {
public:
	LuaPlugin(std::shared_ptr<sol::state> lua, std::string filename, std::string name) :
		m_lua(lua),
		m_env((*lua), sol::create, lua->globals()),
		m_filename(filename),
		m_name(name)
	{}

	virtual std::string GetName() override { return m_name; }
	sol::environment GetEnv() { return m_env; }
	virtual void Init() override;
	virtual void Tick() override;

private:
	std::shared_ptr<sol::state> m_lua;
	sol::environment m_env;
	std::string m_filename;
	std::string m_name;
	sol::function m_Tick;
};

class PluginHandler final {
public:

	void InitLua();

	IPlugin* GetPlugin(std::string name)
	{
		for (auto& obj : m_plugins) {
			if (obj->GetName() == name)
				return obj.get();
		}
		return nullptr;
	}
	void LoadPlugins();
	void ReloadPlugins();
	void AddPlugin(std::unique_ptr<IPlugin> plugin);

	void Update();

private:
	std::shared_ptr<sol::state> m_lua;
	std::vector<std::unique_ptr<IPlugin>> m_plugins;
};

#endif // LUAPLUGINS_H_