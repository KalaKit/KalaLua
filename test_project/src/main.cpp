//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <iostream>
#include <functional>
#include <filesystem>
#include <string>

#include "core/kl_lua.hpp"

using KalaLua::Core::Lua;

using std::cout;
using std::cin;
using std::function;
using std::filesystem::current_path;
using std::filesystem::path;
using std::string;
using std::string_view;
using std::to_string;
using std::setprecision;

int main()
{
	Lua::Initialize();

	Lua::RegisterFunction(
		"double_add",
		"kalalua_test",
		function<void(double, double)>([](double a, double b)
			{ 
				double result = a + b;
				cout << "[CPP] added double b to double a: '" << setprecision(15) << result << "'\n";
			}));

	Lua::RegisterFunction(
		"float_subtract",
		"kalalua_test",
		function<void(float, float)>([](float a, float b)
			{
				float result = a - b;
				cout << "[CPP] subtracted float b from float a: '" << setprecision(6) << result << "'\n";
			}));

	Lua::RegisterFunction(
		"int_multiply",
		"kalalua_test",
		function<void(int, int)>([](int a, int b)
			{
				int result = a * b;
				cout << "[CPP] multiplied int a by int b: '" << setprecision(0) << result << "'\n";
			}));

	Lua::RegisterFunction(
		"double_divide",
		"kalalua_test",
		function<void(double, double)>([](double a, double b)
			{
				double result = a / b;
				cout << "[CPP] divided double a by double b: '" << setprecision(15) << result << "'\n";
			}));

	Lua::RegisterFunction(
		"string_print",
		"kalalua_test",
		function<void(string, string)>([](string a, string b)
			{
				string result = a + ", " + b;
				cout << "[CPP] added string b to string a: '" << result << "'\n";
			}));

	Lua::LoadScript(
		{
			path(current_path() / "files" / "scripts" / "test.lua").string()
		});

	Lua::CallFunction("luaHello", "");

	Lua::CallFunction("bool_compare", "", { 550.4f, 180.565f });

	cout << "ready...\n";

	cin.get();

	cout << "shutting down...\n";

	Lua::Shutdown();

	return 0;
}