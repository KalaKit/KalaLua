//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

#include <string>
#include <functional>
#include <vector>

extern "C"
{
#include "lua.h"
}

#include "KalaHeaders/core_utils.hpp"

namespace KalaLua::Core
{
	using std::string;
	using std::function;
	using std::vector;

	class LIB_API Lua
	{
	public:
		//Initialize KalaLua, does not load scripts or functions
		static bool Initialize();

		//Load and compile a lua script, loaded scripts are overwritten
		static bool LoadScript(const string& script);

		//Load a function with a specific name and in your chosen namespace,
		//functions with the same name under the same namespace are overwritten
		static bool LoadFunction(
			const string& functionName,
			const string& functionNamespace,
			const function<void()>& targetFunction);

		//Call a loaded function
		static bool CallFunction(const string& function);

		static inline bool IsInitialized() { return isInitialized; };
		static inline lua_State* GetLuaState() { return isInitialized ? state : nullptr; }

		//Shut down KalaLua and the Lua runtime
		static void Shutdown();
	private:
		static inline bool isInitialized{};
		static inline lua_State* state{};
	};
}