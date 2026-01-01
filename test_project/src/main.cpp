//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>

#include "core/kl_lua.hpp"

using KalaLua::Core::Lua;

using std::cout;
using std::cin;

int main()
{
	Lua::Initialize();

	lua_State* state = Lua::GetLuaState();

	cout << "ready...\n";

	cin.get();

	cout << "shutting down...\n";

	Lua::Shutdown();

	return 0;
}