//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>
#include <functional>
#include <filesystem>

#include "core/kl_lua.hpp"

using KalaLua::Core::Lua;

using std::cout;
using std::cin;
using std::function;
using std::filesystem::current_path;
using std::filesystem::path;

int main()
{
	Lua::Initialize();

	Lua::RegisterFunction(
		"hello",
		"Test",
		function<void()>([]() 
			{ 
				cout << "[CPP] hello from c++ function\n"; 
			}));

	Lua::LoadScript(
		{
			path(current_path() / "files" / "scripts" / "test.lua").string()
		});

	Lua::CallFunction("luaHello", "");

	Lua::CallFunction("mathtest", "", { 1, 2 });

	cout << "ready...\n";

	cin.get();

	cout << "shutting down...\n";

	Lua::Shutdown();

	return 0;
}