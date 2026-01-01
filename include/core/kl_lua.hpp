//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once

extern "C"
{
#include "lua.h"
}

#include "KalaHeaders/core_utils.hpp"

namespace KalaLua::Core
{
	class LIB_API Lua
	{
	public:
		//Initialize KalaLua, this will internally
		//assign a panic function, open necessary Lua libraries
		//and load the KalaLua api for Lua use
		static bool Initialize();

		static inline bool IsInitialized() { return isInitialized; };
		static inline lua_State* GetLuaState() { return state; }

		//Shut down KalaLua and the Lua runtime
		static void Shutdown();
	private:
		static inline bool isInitialized{};
		static inline lua_State* state{};
	};
}