//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <filesystem>

#include "core/kl_lua.hpp"

using KalaLua::Core::Lua;

using std::cout;
using std::cin;
using std::vector;
using std::string;
using std::function;
using std::filesystem::current_path;
using std::filesystem::path;

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

	Lua::CallFunction("luaHello");

	cout << "ready...\n";

	cin.get();

	cout << "shutting down...\n";

	Lua::Shutdown();

	return 0;
}

vector<LuaFunction> GetFunctions()
{
	vector<LuaFunction> functions{};

	functions.push_back(
	{
		"hello",
		"Test",
		[]()
		{
			cout << "[CPP] hello from c++ function\n";
		}
	});

	return functions;
}

vector<string> GetScripts()
{
	vector<string> scripts{};

	path luaFolder = path(current_path() / "files" / "scripts");

	scripts.push_back(path(luaFolder / "test.lua").string());

	return scripts;
}