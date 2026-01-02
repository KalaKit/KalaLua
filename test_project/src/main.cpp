//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>
#include <vector>
#include <string>
#include <functional>

#include "core/kl_lua.hpp"

using KalaLua::Core::Lua;

using std::cout;
using std::cin;
using std::vector;
using std::string;
using std::function;

struct LuaFunction
{
	string functionName{};
	string functionNamespace{};
	function<void()> targetFunction{};
};

static vector<LuaFunction> GetFunctions();
static vector<string> GetScripts();

int main()
{
	Lua::Initialize();

	for (const auto& f : GetFunctions())
	{
		Lua::LoadFunction(
			f.functionName,
			f.functionNamespace,
			f.targetFunction);
	}

	for (const auto& s : GetScripts())
	{
		Lua::LoadScript(s);
	}

	cout << "ready...\n";

	cin.get();

	cout << "shutting down...\n";

	Lua::Shutdown();

	return 0;
}

vector<LuaFunction> GetFunctions()
{
	vector<LuaFunction> functions{};

	return functions;
}

vector<string> GetScripts()
{
	vector<string> scripts{};

	return scripts;
}