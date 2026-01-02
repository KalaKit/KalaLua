//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <filesystem>
#include <vector>
#include <functional>

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
using std::filesystem::path;
using std::filesystem::exists;
using std::filesystem::is_regular_file;
using std::vector;
using std::function;

static int LuaPanic(lua_State* state);

static int LuaFunctionTrampoline(lua_State* state);

constexpr string_view KALANAMESPACE = "KalaLua";

static vector<function<void()>*> loadedFunctions{};

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

		isInitialized = true;

		Log::Print(
			"Finished initializing KalaLua!",
			"KALALUA_INIT",
			LogType::LOG_SUCCESS);

		return true;
	}

	bool Lua::LoadScript(const string& script)
	{
		if (!isInitialized)
		{
			Log::Print(
				"Failed to load script '" + script + "' because KalaLua is not initialized!",
				"KALALUA_INIT",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		if (!state)
		{
			Log::Print(
				"Failed to load script '" + script + "' because KalaLua state is invalid!",
				"KALALUA_INIT",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		if (!exists(script))
		{
			Log::Print(
				"Failed to load script '" + script + "' because it does not exist!",
				"KALALUA_SCRIPT",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		if (!is_regular_file(script))
		{
			Log::Print(
				"Failed to load script '" + script + "' because it is not a regular file!",
				"KALALUA_SCRIPT",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		if (path(script).extension() != ".lua")
		{
			Log::Print(
				"Failed to load script '" + script + "' because its extension is incorrect!",
				"KALALUA_SCRIPT",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		//load (compile) the script

		int status = luaL_loadfile(state, script.c_str());
		if (status != LUA_OK)
		{
			const char* err = lua_tostring(state, -1);
			string errValue = err ? err : "Unknown error.";

			Log::Print(
				"Lua load error in script '" + script + "': " + errValue,
				"KALALUA_SCRIPT",
				LogType::LOG_ERROR,
				2);

			lua_pop(state, 1);

			return false;
		}

		//execute the script

		status = lua_pcall(
			state,
			0,
			LUA_MULTRET,
			0);
		if (status != LUA_OK)
		{
			const char* err = lua_tostring(state, -1);
			string errValue = err ? err : "Unknown error.";

			Log::Print(
				"Lua runtime error in script '" + script + "': " + errValue,
				"KALALUA_SCRIPT",
				LogType::LOG_ERROR,
				2);

			lua_pop(state, 1);

			return false;
		}

		Log::Print(
			"Loaded script '" + script + "'!",
			"KALALUA_SCRIPT",
			LogType::LOG_SUCCESS);

		return true;
	}

	bool Lua::LoadFunction(
		const string& functionName,
		const string& functionNamespace,
		const function<void()>& targetFunction)
	{
		if (!isInitialized)
		{
			Log::Print(
				"Failed to load function '" + functionName + "' because KalaLua is not initialized!",
				"KALALUA_LOAD_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		if (functionName.empty()
			|| functionName.size() > 50)
		{
			Log::Print(
				"Function with empty or too long name was skipped.",
				"KALALUA_LOAD_FUNCTION",
				LogType::LOG_WARNING);

			return false;
		}
		if (functionNamespace.empty()
			|| functionNamespace.size() > 50)
		{
			Log::Print(
				"Function namespace for function '" + functionName + "' was empty or too long and was skipped.",
				"KALALUA_LOAD_FUNCTION",
				LogType::LOG_WARNING);

			return false;
		}
		if (targetFunction == nullptr)
		{
			Log::Print(
				"Function target for function '" + functionName + "' was empty and was skipped.",
				"KALALUA_LOAD_FUNCTION",
				LogType::LOG_WARNING);

			return false;
		}

		lua_getglobal(state, functionNamespace.c_str());
		if (!lua_istable(state, -1))
		{
			lua_pop(state, 1);
			lua_newtable(state);
			lua_pushvalue(state, -1);
			lua_setglobal(state, functionNamespace.c_str());
		}

		auto* storedf = new function<void()>(targetFunction);
		loadedFunctions.push_back(storedf);

		//push user data (upvalue)
		lua_pushlightuserdata(state, storedf);

		//create closure with 1 upvalue
		lua_pushcclosure(state, LuaFunctionTrampoline, 1);

		//set global function name
		lua_setfield(state, -2, functionName.c_str());

		//pop namespace table
		lua_pop(state, 1);

		return true;
	}

	bool Lua::CallFunction(const string& function)
	{
		if (!isInitialized)
		{
			Log::Print(
				"Failed to call function '" + function + "' because KalaLua is not initialized!",
				"KALALUA_CALL_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		lua_getglobal(state, function.c_str());

		if (!lua_isfunction(state, -1))
		{
			lua_pop(state, 1);

			Log::Print(
				"Lua function '" + function + "' does not exist!",
				"KALALUA_CALL_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		if (lua_pcall(state, 0, 0, 0) != LUA_OK)
		{
			const char* err = lua_tostring(state, -1);
			string errValue = err ? err : "Unknown error.";

			Log::Print(
				"Lua runtime error with function '" + function + "': " + errValue,
				"KALALUA_CALL_FUNCTION",
				LogType::LOG_ERROR,
				2);

			lua_pop(state, 1);

			return false;
		}

		Log::Print(
			"Called function '" + function + "'.",
			"KALALUA_CALL_FUNCTION",
			LogType::LOG_DEBUG);

		return true;
	}

	void Lua::Shutdown()
	{
		if (!isInitialized) return;

		Log::Print(
			"Shutting down KalaLua.",
			"KALALUA_SHUTDOWN",
			LogType::LOG_INFO);

		for (auto* f : loadedFunctions) delete f;
		loadedFunctions.clear();

		lua_close(state);
		state = nullptr;
		isInitialized = false;
	}
}

int LuaPanic(lua_State* state)
{
	const char* msg = lua_tostring(state, -1);

	KalaLuaCore::ForceClose(
		"KalaLua error",
		msg ? msg : "Unknown lua panic.");

	return 0;
}

int LuaFunctionTrampoline(lua_State* state)
{
	auto* f = scast<function<void()>*>(lua_touserdata(state, lua_upvalueindex(1)));

	if (!f)
	{
		return luaL_error(
			state,
			"KALALUA ERROR: User-defined function has no target function!");
	}

	//call the function
	(*f)();
	
	return 0;
}