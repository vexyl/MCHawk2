#include "../include/LuaPlugins.hpp"
#include "../include/Server.hpp"
#include "../include/Utils/Vector.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <filesystem>

#undef SendMessage

namespace LuaPlugins {
	std::vector<sol::function> authEvent;
	std::vector<sol::function> messageEvent;
	std::vector<sol::function> joinEvent;
	std::vector<sol::function> positionOrientationEvent;
	std::vector<sol::function> setBlockEvent;
	std::vector<sol::function> disconnectEvent;
	std::vector<sol::function> playerClickedEvent;
} // namespace LuaPlugins

void LuaPlugin::Init()
{
	m_lua->script_file(m_filename, m_env);

	sol::function init = m_env["Init"];
	if (!init.valid())
		throw std::runtime_error(std::string("LuaPlugin: " + m_name + " (" + m_filename + ") Init function not found"));

	sol::function tick = m_env["Tick"];
	if (tick.valid())
		m_tick = tick;

	try {
		init();
	}
	catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	}
}

void LuaPlugin::Tick()
{
	if (m_tick != sol::nil)
		m_tick();
}

void PluginHandler::InitLua()
{
	m_lua = std::make_shared<sol::state>();
	m_lua->open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::table, sol::lib::math, sol::lib::os, sol::lib::debug);

	m_lua->new_usertype<Net::Client>("Client", "GetID", &Net::Client::GetID);

	m_lua->new_usertype<Player>("Player",
		"GetName", &Player::GetName,
		"GetWorld", &Player::GetWorld,
		"GetClient", &Player::GetClient,
		"GetID", &Player::GetID,
		"GetPosition", &Player::GetPosition,
		"SetPosition", &Player::SetPosition,
		"SendMessage", &Player::SendMessage
	);

	m_lua->new_usertype<World>("World",
		"GetName", &World::GetName,
		"AddPlayer", &World::AddPlayer,
		"RemovePlayer", &World::RemovePlayer,
		"GetPlayerPids", &World::GetPlayerPids,
		"GetSpawnPosition", &World::GetSpawnPosition,
		"GetMap", &World::GetMap
	);

	m_lua->new_usertype<Map>("Map",
		"PeekBlock", &Map::PeekBlock
	);

	m_lua->new_usertype<Utils::Vector>("Vector",
		sol::constructors<Utils::Vector(float, float, float)>(),
		"Normalized", &Utils::Vector::Normalized,
		"x", sol::property(&Utils::Vector::SetX, &Utils::Vector::GetX),
		"y", sol::property(&Utils::Vector::SetY, &Utils::Vector::GetY),
		"z", sol::property(&Utils::Vector::SetZ, &Utils::Vector::GetZ)
	);

	(*m_lua)["GetPlugin"] = [&](std::string name) {
		return dynamic_cast<LuaPlugin*>(GetPlugin(name))->GetEnv();
	};

	m_lua->set_function("RegisterEvent", [&](std::string name, sol::function func)
		{
			if (name == "OnAuthentication")
				LuaPlugins::authEvent.push_back(func);
			else if (name == "OnMessage")
				LuaPlugins::messageEvent.push_back(func);
			else if (name == "OnJoin")
				LuaPlugins::joinEvent.push_back(func);
			else if (name == "OnPositionOrientation")
				LuaPlugins::positionOrientationEvent.push_back(func);
			else if (name == "OnSetBlock")
				LuaPlugins::setBlockEvent.push_back(func);
			else if (name == "OnDisconnect")
				LuaPlugins::disconnectEvent.push_back(func);
			else if (name == "OnPlayerClicked")
				LuaPlugins::playerClickedEvent.push_back(func);
			else
				LOG(LOGLEVEL_WARNING, "RegisterEvent: Invalid event %s", name.c_str());
		}
	);

	(*m_lua)["ReloadPlugins"] = [&]() { ReloadPlugins(); };
	(*m_lua)["BlockDefaultEventHandler"] = [&]() { Server::GetInstance()->BlockDefaultEventHandler(); };
	(*m_lua)["GetWorld"] = [&](std::string name) { return Server::GetInstance()->GetWorld(name); };
	(*m_lua)["GetPlayer"] = [&](int8_t pid) { return Server::GetInstance()->GetPlayer(pid); };
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
	LuaPlugins::authEvent.clear();
	LuaPlugins::messageEvent.clear();
	LuaPlugins::joinEvent.clear();
	LuaPlugins::positionOrientationEvent.clear();
	LuaPlugins::setBlockEvent.clear();
	LuaPlugins::disconnectEvent.clear();
	LuaPlugins::playerClickedEvent.clear();

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

void PluginHandler::TriggerAuthEvent(Player::PlayerPtr player)
{
	try {
		for (auto& func : LuaPlugins::authEvent)
			func(player);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerAuthEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerMessageEvent(Player::PlayerPtr player, std::string message, uint8_t flag)
{
	sol::table table = m_lua->create_table_with("message", message, "flag", flag);
	try {
		for (auto& func : LuaPlugins::messageEvent)
			func(player, table);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerMessageEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerJoinEvent(Player::PlayerPtr player)
{
	try {
		for (auto& func : LuaPlugins::joinEvent)
			func(player);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerJoinEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerPositionOrientationEvent(Player::PlayerPtr player, Utils::Vector position, uint8_t yaw, uint8_t pitch)
{
	sol::table table = m_lua->create_table_with("position", position, "yaw", yaw, "pitch", pitch);
	try {
		for (auto& func : LuaPlugins::positionOrientationEvent)
			func(player, table);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerPositionOrientationEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerSetBlockEvent(Player::PlayerPtr player, int blockType, Utils::Vector position)
{
	try {
		for (auto& func : LuaPlugins::setBlockEvent)
			func(player, blockType, position);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerSetBlockEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerDisconnectEvent(Player::PlayerPtr player)
{
	try {
		for (auto& func : LuaPlugins::disconnectEvent)
			func(player);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerDisconnectEvent exception: " << e.what() << std::endl;
	}
}

void PluginHandler::TriggerPlayerClickedEvent(Player::PlayerPtr player, int8_t targetEntityID)
{
	try {
		for (auto& func : LuaPlugins::playerClickedEvent)
			func(player, targetEntityID);
	}
	catch (std::runtime_error& e) {
		std::cerr << "TriggerPlayerClickedEvent exception: " << e.what() << std::endl;
	}
}