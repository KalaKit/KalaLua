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
#include "KalaHeaders/string_utils.hpp"

#include "core/kl_lua.hpp"
#include "core/kl_core.hpp"

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;
using KalaHeaders::KalaString::ContainsString;
using KalaHeaders::KalaString::SplitString;

using KalaLua::Core::Lua;
using KalaLua::Core::LuaVar;
using KalaLua::Core::KalaLuaCore;

using std::string;
using std::string_view;
using std::to_string;
using std::filesystem::path;
using std::filesystem::exists;
using std::filesystem::is_regular_file;
using std::vector;
using std::function;
using std::visit;
using std::decay_t;
using std::is_same_v;

static int LuaPanic(lua_State* state);

static int LuaFunctionTrampolineArgs(lua_State* state);
static int LuaFunctionTrampolineCustom(lua_State* state);

static vector<function<void(const vector<LuaVar>&)>*> loadedArgFunctions{};
static vector<function<int(lua_State*)>*> loadedCustomFunctions{};

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

	bool Lua::CallFunction(
		const string& functionName,
		const string& functionNamespace,
		const vector<LuaVar>& args)
	{
		if (functionName.empty())
		{
			Log::Print(
				"Failed to call function because name was empty!",
				"KALALUA_CALL_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		if (!isInitialized)
		{
			Log::Print(
				"Failed to call function '" + functionName + "' because KalaLua is not initialized!",
				"KALALUA_CALL_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		if (functionNamespace.empty())
		{
			lua_pushglobaltable(state);
		}
		else if (functionNamespace.find('.') == string::npos)
		{
			lua_getglobal(state, functionNamespace.c_str());

			if (!lua_istable(state, -1))
			{
				lua_pop(state, 1);

				Log::Print(
					"Lua namespace '" + functionNamespace + "' does not exist!",
					"KALALUA_CALL_FUNCTION",
					LogType::LOG_ERROR,
					2);

				return false;
			}
		}
		else
		{
			const vector<string> parts = SplitString(functionNamespace, ".");

			lua_pushglobaltable(state);

			for (const auto& p : parts)
			{
				lua_getfield(state, -1, p.c_str());
				lua_remove(state, -2);

				if (lua_isnil(state, -1))
				{
					lua_pop(state, 1);

					Log::Print(
						"Lua namespace '" + functionNamespace + "' does not exist!",
						"KALALUA_CALL_FUNCTION",
						LogType::LOG_ERROR,
						2);

					return false;
				}
			}
		}

		//fetch function from resolved namespace
		lua_getfield(state, -1, functionName.c_str());
		lua_remove(state, -2);

		if (!lua_isfunction(state, -1))
		{
			lua_pop(state, 1);

			Log::Print(
				"Lua function '" + functionName + "' does not exist!",
				"KALALUA_CALL_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		//push arguments
		for (const LuaVar& v : args)
		{
			visit([](auto&& value)
				{
					using T = decay_t<decltype(value)>;

					if constexpr (is_same_v<T, int>)         lua_pushinteger(state, value);
					else if constexpr (is_same_v<T, float>)  lua_pushnumber(state, value);
					else if constexpr (is_same_v<T, double>) lua_pushnumber(state, value);
					else if constexpr (is_same_v<T, bool>)   lua_pushboolean(state, value);
					else if constexpr (is_same_v<T, string>) lua_pushstring(state, value.c_str());
				}, v);

		}

		int status = lua_pcall(
			state,
			scast<int>(args.size()),
			0,
			0);

		if (status != LUA_OK)
		{
			const char* err = lua_tostring(state, -1);
			string errValue = err ? err : "Unknown error";

			Log::Print(
				"Lua runtime error with function '" + functionName + "': " + errValue,
				"KALALUA_CALL_FUNCTION",
				LogType::LOG_ERROR,
				2);

			lua_pop(state, 1);

			return false;
		}

		if (functionNamespace.empty())
		{
			Log::Print(
				"Called global function '" 
				+ functionName + "' with '" 
				+ to_string(args.size()) + "' args.",
				"KALALUA_CALL_FUNCTION",
				LogType::LOG_DEBUG);
		}
		else
		{
			Log::Print(
				"Called function '" 
				+ functionName + "' in namespace '" 
				+ functionNamespace + "' with '" 
				+ to_string(args.size()) + "' args.",
				"KALALUA_CALL_FUNCTION",
				LogType::LOG_DEBUG);
		}

		return true;
	}

	bool Lua::RegisterFunction(
		const string& functionName,
		const string& functionNamespace,
		const function<int(lua_State*)>& targetFunction)
	{
		if (functionName.empty()
			|| functionName.size() > 50)
		{
			Log::Print(
				"Failed to register function because name was empty or too long.",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}
		if (functionNamespace.size() > 50)
		{
			Log::Print(
				"Failed to register function '" + functionName + "' because namespace was too long.",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}
		if (!targetFunction)
		{
			Log::Print(
				"Failed to register function '" + functionName + "' because target function was empty.",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		if (!isInitialized)
		{
			Log::Print(
				"Failed to register function '" + functionName + "' because KalaLua is not initialized!",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		//no namespace
		if (functionNamespace.empty())
		{
			lua_pushglobaltable(state);
		}
		//namespace has no dots
		else if (functionNamespace.find('.') == string::npos)
		{
			lua_getglobal(state, functionNamespace.c_str());

			if (!lua_istable(state, -1))
			{
				lua_pop(state, 1);
				lua_newtable(state);
				lua_pushvalue(state, -1);
				lua_setglobal(state, functionNamespace.c_str());
			}
		}
		//namespace has dots
		else
		{
			const vector<string> parts = SplitString(functionNamespace, ".");

			lua_pushglobaltable(state);

			for (const auto& p : parts)
			{
				lua_getfield(state, -1, p.c_str());

				if (!lua_istable(state, -1))
				{
					lua_pop(state, 1);
					lua_newtable(state);
					lua_pushvalue(state, -1);
					lua_setfield(state, -3, p.c_str());
				}

				//remove parent table
				lua_remove(state, -2);
			}
		}

		auto* storedf = new function<int(lua_State*)>(targetFunction);
		loadedCustomFunctions.push_back(storedf);

		//push user data (upvalue)
		lua_pushlightuserdata(state, storedf);

		//create closure with 1 upvalue
		lua_pushcclosure(state, LuaFunctionTrampolineCustom, 1);

		//set global function name
		lua_setfield(state, -2, functionName.c_str());

		//pop namespace table
		lua_pop(state, 1);

		if (!functionNamespace.empty())
		{
			Log::Print(
				"Registered function '" + functionName + "' to namespace '" + functionNamespace + "'!",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_SUCCESS);
		}
		else
		{
			Log::Print(
				"Registered function '" + functionName + "' to global namespace!",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_SUCCESS);
		}

		return true;
	}

	bool Lua::_RegisterFunction(
		const string& functionName,
		const string& functionNamespace,
		function<void(const vector<LuaVar>&)> invoker,
		size_t argCount)
	{
		if (functionName.empty()
			|| functionName.size() > 50)
		{
			Log::Print(
				"Failed to register function because name was empty or too long.",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}
		if (functionNamespace.size() > 50)
		{
			Log::Print(
				"Failed to register function '" + functionName + "' because namespace was too long.",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		if (!isInitialized)
		{
			Log::Print(
				"Failed to register function '" + functionName + "' because KalaLua is not initialized!",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_ERROR,
				2);

			return false;
		}

		//no namespace
		if (functionNamespace.empty())
		{
			lua_pushglobaltable(state);
		}
		//namespace has no dots
		else if (functionNamespace.find('.') == string::npos)
		{
			lua_getglobal(state, functionNamespace.c_str());

			if (!lua_istable(state, -1))
			{
				lua_pop(state, 1);
				lua_newtable(state);
				lua_pushvalue(state, -1);
				lua_setglobal(state, functionNamespace.c_str());
			}
		}
		//namespace has dots
		else
		{
			const vector<string> parts = SplitString(functionNamespace, ".");

			lua_pushglobaltable(state);

			for (const auto& p : parts)
			{
				lua_getfield(state, -1, p.c_str());

				if (!lua_istable(state, -1))
				{
					lua_pop(state, 1);
					lua_newtable(state);
					lua_pushvalue(state, -1);
					lua_setfield(state, -3, p.c_str());
				}

				//remove parent table
				lua_remove(state, -2);
			}
		}

		auto* storedf = new function<void(const vector<LuaVar>&)>(move(invoker));
		loadedArgFunctions.push_back(storedf);

		//push user data (upvalue)
		lua_pushlightuserdata(state, storedf);

		//create closure with 1 upvalue
		lua_pushcclosure(state, LuaFunctionTrampolineArgs, 1);

		//set global function name
		lua_setfield(state, -2, functionName.c_str());

		//pop namespace table
		lua_pop(state, 1);

		if (!functionNamespace.empty())
		{
			Log::Print(
				"Registered function '" + functionName + "' to namespace '" + functionNamespace + "'!",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_SUCCESS);
		}
		else
		{
			Log::Print(
				"Registered function '" + functionName + "' to global namespace!",
				"KALALUA_REGISTER_FUNCTION",
				LogType::LOG_SUCCESS);
		}

		return true;
	}

	void Lua::Shutdown()
	{
		if (!isInitialized) return;

		Log::Print(
			"Shutting down KalaLua.",
			"KALALUA_SHUTDOWN",
			LogType::LOG_INFO);

		for (auto* f : loadedArgFunctions) delete f;
		loadedArgFunctions.clear();

		for (auto* f : loadedCustomFunctions) delete f;
		loadedCustomFunctions.clear();

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

int LuaFunctionTrampolineArgs(lua_State* state)
{
	auto* f = scast<function<void(const vector<LuaVar>&)>*>(lua_touserdata(state, lua_upvalueindex(1)));

	if (!f)
	{
		return luaL_error(
			state,
			"KALALUA ERROR: User passed function whose target action was invalid!");
	}

	const int argc = lua_gettop(state);
	vector<LuaVar> args{};
	args.reserve(argc);

	for (int i = 1; i <= argc; ++i)
	{
		switch (lua_type(state, i))
		{
		case LUA_TNUMBER:
			args.emplace_back(scast<double>(lua_tonumber(state, i)));
			break;
		case LUA_TBOOLEAN:
			args.emplace_back(scast<bool>(lua_toboolean(state, i)));
			break;
		case LUA_TSTRING:
			args.emplace_back(string(lua_tostring(state, i)));
			break;
		default:
			return luaL_error(
				state,
				"KALALUA ERROR: User passed unsupported types to LoadFunction!");
		}
	}

	//call the function
	(*f)(args);
	
	return 0;
}

int LuaFunctionTrampolineCustom(lua_State* state)
{
	auto* f = scast<function<int(lua_State*)>*>(lua_touserdata(state, lua_upvalueindex(1)));

	if (!f)
	{
		return luaL_error(
			state,
			"KALALUA ERROR: User-defined function has no target function!");
	}

	//call the function
	return (*f)(state);
}