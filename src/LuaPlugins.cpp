#include "../include/LuaPlugins.hpp"
#include "../include/Server.hpp"
#include "../include/Utils/Event.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <filesystem>

void LuaPlugin::Init()
{
	m_lua->script_file(m_filename, m_env);

	sol::function init = m_env["Init"];
	if (!init.valid())
		throw std::runtime_error(std::string("LuaPlugin: " + m_name + " (" + m_filename + ") Init function not found"));

	sol::function tick = m_env["Tick"];
	if (tick.valid())
		m_Tick = tick;

	try {
		init();
	}
	catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	}
}

void LuaPlugin::Tick()
{
	if (m_Tick != sol::nil)
		m_Tick();
}

void PluginHandler::InitLua()
{
	m_lua = std::make_shared<sol::state>();
	m_lua->open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::table, sol::lib::math, sol::lib::os, sol::lib::debug);

	(*m_lua)["GetPlugin"] = [&](std::string name) {
		return dynamic_cast<LuaPlugin*>(GetPlugin(name))->GetEnv();
	};
}

void PluginHandler::LoadPlugins()
{
	std::string pluginDir = "plugins";
	for (auto iter = std::filesystem::recursive_directory_iterator(pluginDir);
			iter != std::filesystem::recursive_directory_iterator();
			++iter) {
		const std::string filename = iter->path().string();
		std::string pluginName = iter->path().parent_path().filename().string();
		if (iter->path().filename().string() == "init.lua") {
			std::unique_ptr<IPlugin> plugin = std::make_unique<LuaPlugin>(m_lua, filename, pluginName);
			AddPlugin(std::move(plugin));
		}
	}

	for (auto& plugin : m_plugins) {
		try {
			plugin->Init();
			LOG(LOGLEVEL_DEBUG, "Loaded plugin: %s", plugin->GetName());
		}
		catch (const std::runtime_error& e) {
			LOG(LOGLEVEL_WARNING, "PluginHandler error (%s): %s", plugin->GetName(), e.what());
		}
	}
}

void PluginHandler::ReloadPlugins()
{
	m_plugins.clear();
	m_lua.reset();
	InitLua();
	LoadPlugins();
}

void PluginHandler::AddPlugin(std::unique_ptr<IPlugin> plugin)
{
	m_plugins.push_back(std::move(plugin));
}

void PluginHandler::Update()
{
	// TODO: update at regular intervals (tick rate)
	for (auto& plugin : m_plugins) {
		try {
			plugin->Tick();
		} catch(std::runtime_error& e) {
			std::cerr << e.what() << std::endl;
		}
	}
}