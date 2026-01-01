//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <string>

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "KalaHeaders/log_utils.hpp"

#include "core/kl_lua.hpp"
#include "core/kl_core.hpp"

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using KalaLua::Core::Lua;
using KalaLua::Core::KalaLuaCore;

using std::string_view;
using std::string;

static int LuaPanic(lua_State* state);
static void RegisterAPI(lua_State* state);

constexpr string_view KALANAMESPACE = "KalaLua";

namespace KalaLua::Core
{
	bool Lua::Initialize()
	{
		if (isInitialized)
		{
			Log::Print(
				"Failed to initialize KalaLua because its already initialized!",
				"KALALUA_INIT",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		state = luaL_newstate();
		if (!state)
		{
			Log::Print(
				"Failed to initialize KalaLua because its state couldn't be created!",
				"KALALUA_INIT",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		luaL_openlibs(state);
		lua_atpanic(state, LuaPanic);

		RegisterAPI(state);

		isInitialized = true;

		Log::Print(
			"Finished initializing KalaLua!",
			"KALALUA_INIT",
			LogType::LOG_SUCCESS);

		return true;
	}

	void Lua::Shutdown()
	{
		if (!isInitialized) return;

		Log::Print(
			"Shutting down KalaLua.",
			"KALALUA_SHUTDOWN",
			LogType::LOG_INFO);

		lua_close(state);
		state = nullptr;
		isInitialized = false;
	}
}

int LuaPanic(lua_State* state)
{
	const char* msg = lua_tostring(state, -1);

	KalaLuaCore::ForceClose(
		"Lua has panicked",
		msg ? msg : "Unknown lua panic.");

	return 0;
}

void RegisterAPI(lua_State* state)
{
	if (!state)
	{
		Log::Print(
			"Failed to register KalaLua API because its state was invalid!",
			"KALALUA_API",
			LogType::LOG_ERROR,
			2);

		return;
	}

	lua_newtable(state);

	//api here...

	lua_setglobal(state, KALANAMESPACE.data());
}